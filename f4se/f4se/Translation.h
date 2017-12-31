#pragma once

#include "F4SE/ScaleformLoader.h"
#include "F4SE/ScaleformTranslator.h"

namespace Translation
{
	void ImportTranslationFiles(BSScaleformTranslator * translator);

	void ParseTranslation(BSScaleformTranslator * translator, std::string & name);
}
