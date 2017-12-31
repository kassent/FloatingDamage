#include "Console.h"
#include "F4SE/ObScript.h"
#include "F4SE/GameData.h"
#include "F4SE/GameReferences.h"
#include "F4SE/GameAPI.h"
#include "F4SE/GameRTTI.h"
#include "F4SE_common/SafeWrite.h"

#include <vector>

bool DumpClass_Execute(const ObScriptParam * paramInfo, ObScriptCommand::ScriptData * scriptData, TESObjectREFR * thisObj, void * containingObj, void * scriptObj, void * locals, double& result, UInt32& opcodeOffsetPtr)
{
	ObScriptCommand::StringChunk *strChunk = (ObScriptCommand::StringChunk*)scriptData->GetChunk();

	std::string target = strChunk->GetString();

	char * pTerminal = &target[target.size() - 1];

	long iTarget = strtol(&target[0], &pTerminal, 0x10);

	ObScriptCommand::IntegerChunk *intChunk = (ObScriptCommand::IntegerChunk*)strChunk->GetNext();
	int val = intChunk->GetInteger();

	_MESSAGE("%s   %08X   %d", __FUNCTION__, iTarget, val);

	DumpClass(reinterpret_cast<void*>(iTarget), val);

	return true;
}
#include <vector>
class PerkVisitor
{
public:
	struct Entry
	{
		BGSPerk*	perk;
		UInt8		rank;
	};
	virtual bool Accept(Entry* pEntry) = 0;
};

class HasPerkVisitor
{
public:
	struct Entry
	{
		BGSPerk*	perk;
		UInt8		rank;
	};
	virtual uintptr_t Accept(Entry* pEntry) //bool don't use bool,VS2015 compiler seems has a bug.
	{
		if (pEntry != nullptr && pEntry->perk != nullptr)
		{
			result.push_back(Entry{ pEntry->perk, pEntry->rank });
		}
		return 1;
	}

	std::vector<Entry>				result;
};


bool DumpConsoleTarget_Execute(const ObScriptParam * paramInfo, ObScriptCommand::ScriptData * scriptData, TESObjectREFR * thisObj, void * containingObj, void * scriptObj, void * locals, double& result, UInt32& opcodeOffsetPtr)
{
	TESObjectREFR* pRef = nullptr;
	LookupREFRByHandle(g_consoleHandle, &pRef);
	if (pRef != nullptr)
	{
		Console_Print(" ");
		Console_Print("> refFormID:			%08X", pRef->formID);
		Console_Print("> refFormType:		%d", pRef->formType);
		Console_Print("> refAddress:		%016I64X", (uintptr_t)pRef);
		Console_Print("> fullName:			%s", CALL_MEMBER_FN(pRef, GetReferenceName)());
		Console_Print("> refEditorID:		%s", pRef->GetEditorID());

		if (pRef->unk08 != nullptr && pRef->unk08->entries != nullptr)
		{
			for (size_t i = 0; i < pRef->unk08->size; ++i)
			{
				auto pFile = pRef->unk08->entries[i]->file;
				for (const auto& mod : (*g_dataHandler)->modList.loadedMods)
				{
					if (mod != nullptr && mod->file == pFile)
					{
						Console_Print("> modName: %s   modIndex: %d   file: %016I64X", mod->name, mod->modIndex, (uintptr_t)mod->file);
					}
				}
			}
		}


		if (pRef->baseForm != nullptr)
		{
			Console_Print("> baseFormID:		%08X", pRef->baseForm->formID);
			Console_Print("> baseFormType:		%d", pRef->baseForm->formType);
			Console_Print("> baseAddress:		%016I64X", (uintptr_t)pRef->baseForm);
			Console_Print("> baseFormEditorID:	%s", pRef->baseForm->GetEditorID());

			if (pRef->baseForm->unk08 != nullptr && pRef->baseForm->unk08->entries != nullptr)
			{
				for (size_t i = 0; i < pRef->baseForm->unk08->size; ++i)
				{
					auto pFile = pRef->baseForm->unk08->entries[i]->file;
					for (const auto& mod : (*g_dataHandler)->modList.loadedMods)
					{
						if (mod != nullptr && mod->file == pFile)
						{
							Console_Print("> modName: %s   modIndex: %d   file: %016I64X", mod->name, mod->modIndex, (uintptr_t)mod->file);
						}
					}
				}
			}
		}



		if (pRef->formType == kFormType_ACHR)
		{
			Actor* actor = static_cast<Actor*>(pRef);



			RelocAddr<void(*)(Actor*, HasPerkVisitor&)> fnDumpPerks = 0xD8C390;

			HasPerkVisitor visitor;
			//9C6A70
			if (actor->middleProcess != nullptr)
			{
				fnDumpPerks(actor, visitor);

				for (auto& pEntry : visitor.result)
				{
					TESFullName* pFullName = DYNAMIC_CAST(pEntry.perk, BGSPerk, TESFullName);
					Console_Print("> name: %s                                   formID:  %08X		Rank: %d", pFullName->name.c_str(), pEntry.perk->formID, pEntry.rank);
				}
			}
		}

	}

	return true;
}


void RegisterCommands()
{
	RelocAddr<ObScriptCommand*(*)(uint32_t)> fnDumpScriptCommands = 0x04E9720;
	for (uint32_t i = 0; i < 0x1333; ++i)
	{
		ObScriptCommand* pCommand = fnDumpScriptCommands(i);
#ifdef _DEBUG
		if (pCommand != nullptr && pCommand->longName && pCommand->longName[0] && pCommand->opcode)
		{
			_MESSAGE("");
			_MESSAGE("longName : %s", pCommand->longName);
			_MESSAGE("shortName: %s", pCommand->shortName);
			_MESSAGE("opCode:    %d", pCommand->opcode);
			_MESSAGE("command:   %08I64X", (uintptr_t)pCommand - (uintptr_t)GetModuleHandle(NULL));
		}
#endif 
		if (pCommand != nullptr && pCommand->longName && !strcmp(pCommand->longName, "CellInfo"))
		{
			static ObScriptParam params[] = {
				{ "String", ObScriptParam::kType_String, 0 },
				{ "Integer", ObScriptParam::kType_Integer, 0 }
			};

			ObScriptCommand cmd = *pCommand;

			cmd.longName = "DumpClass";
			cmd.shortName = "dc";
			cmd.helpText = "";
			cmd.needsParent = 0;
			cmd.numParams = 2;
			cmd.params = params;
			cmd.execute = (ObScript_Execute)DumpClass_Execute;
			cmd.flags = 0;

			SafeWriteBuf((uintptr_t)pCommand, &cmd, sizeof(cmd));
		}

		if (pCommand != nullptr && pCommand->longName && !strcmp(pCommand->longName, "SetGamma"))
		{
			ObScriptCommand cmd = *pCommand;

			cmd.longName = "DumpConsoleClass";
			cmd.shortName = "dcc";
			cmd.helpText = "";
			cmd.needsParent = 0;
			cmd.numParams = 0;
			cmd.params = nullptr;
			cmd.execute = (ObScript_Execute)DumpConsoleTarget_Execute;
			cmd.flags = 0;

			SafeWriteBuf((uintptr_t)pCommand, &cmd, sizeof(cmd));
		}
	}


}