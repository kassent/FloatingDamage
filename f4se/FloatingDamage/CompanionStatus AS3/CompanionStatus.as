package {
	import flash.events.IOErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.URLLoader;
	import flash.net.URLLoaderDataFormat;
	import flash.net.URLRequest;
	import flash.system.ApplicationDomain;
	import flash.text.TextField;
	import flash.utils.Timer;
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.utils.Dictionary;
	import flash.utils.getDefinitionByName;
	import flash.events.KeyboardEvent;
	import flash.ui.Keyboard;
	
	import Shared.AS3.BSUIComponent;
	
	import hudframework.IHUDWidget;
	
	import r2k.components.*;
	
	/**
	* Manages display of multiple companion status widgets.
	*/
	public class CompanionStatus extends BSUIComponent implements IHUDWidget {
		// Constants
		private static const WIDGET_IDENTIFIER:String = "CompanionStatus";
		private static const NAME_MAP_PATH:String = "Config/CompanionStatus/name_map.ini";
		
		// Command IDs
		private static const Command_SetFollowerName:int 	= 101;
		private static const Command_ShowWidget:int 		= 201;
		private static const Command_HideWidget:int 		= 202;
		private static const Command_UpdateStats:int 		= 301;
		
		// Variables
        private static var Y_SPACING:Number = 12;
		private static var Y_SPAWN_OFFSET:Number = 24;
		
		private var FollowerIDToWidgetMap:Dictionary = new Dictionary(true);
		private var StatusWidgets:Vector.<CompanionStatusFader> = new Vector.<CompanionStatusFader>();
		private var ShownStatusWidgets:Vector.<int> = new Vector.<int>();
		
		// A user-specified follower name mapping.
		// Used for mapping Dog -> Dogmeat, for instance.
		private var FollowerNameMap:Dictionary = new Dictionary(true);
		
		public function CompanionStatus() {
			getHUDFramework();
			this.addEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
			
			// Load name-map file.
			var _nameMapLoader:URLLoader = new URLLoader();
			_nameMapLoader.dataFormat = URLLoaderDataFormat.TEXT;
			_nameMapLoader.addEventListener(Event.COMPLETE, handleNameMapLoaded, false, 0, true);
			_nameMapLoader.addEventListener(IOErrorEvent.IO_ERROR, handleNameMapLoadFailed, false, 0, true);
			
			try {
				log("Loading name mappings...");
				_nameMapLoader.load(new URLRequest(NAME_MAP_PATH));
			} catch (e:Error) {
				log("Exception thrown when trying to load name-map file.");
				log(e.name + " : " + e.message);
				handleNameMapLoadFailed();
			}
		}
		
		private function addedToStageHandler(e:Event):void {
			this.addEventListener(Event.ENTER_FRAME, enterFrameHandler);
			stage.addEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
			
			// Test populating data.
			/*AddFollower(0, "Follower");
			UpdateStats(0, 1.0, 0.75, 20);
			ShowFollower(0);*/
		}
		
		private function handleNameMapLoaded(e:Event):void {
			log("Name-map loaded.");
			processNameMap(e.target.data as String);
		}
		
		private function handleNameMapLoadFailed(e:IOErrorEvent = null) {
			log("Name-map load failed.");
		}
		
		private function processNameMap(str:String):void {
			try {
				var strArr:Array = str.split("\n");
				for (var i:int = 0; i < strArr.length; i++) {
					var mapPair:Array = strArr[i].split("=");
					FollowerNameMap[mapPair[0]] = mapPair[1];
					
					// Run through StatusWidgets in case we were only loaded after the names have been populated.
					for (var j:int = 0; j < StatusWidgets.length; j++) {
						if (StatusWidgets[j].widget.data.name == mapPair[0]) {
							StatusWidgets[j].widget.data.name = mapPair[1];
							StatusWidgets[j].widget.SetIsDirty();
						}
					}
				}
				
			} catch (e:Error) {
				log("Error occurred while parsing name_map.ini.");
				log(e.name + " : " + e.message);
			}
		}
		
		public function getHUDFramework():void {
			var hud:Object;
			if (ApplicationDomain.currentDomain.hasDefinition("hudframework.HUDFramework")) {
				hud = getDefinitionByName("hudframework.HUDFramework")["getInstance"]();
			}
			if (hud) {
				log("HUDFramework is present. Now registered for HUD updates.");
			} else {
				log("HUDFramework is not present. No updates will be received.");
			}
		}
		
		public function processMessage(command:String, params:Array):void {
			//log("Message received.");
			//log("Command: " + command + " Params: " + params.toString());
			switch(command) {
				case String(Command_SetFollowerName):
				case "SetFollowerName":
					AddFollower(int(params[0]), String(params[1]));
					break;
					
				case String(Command_UpdateStats):
				case "UpdateStats":
					UpdateStats(int(params[0]), Number(params[1]), Number(params[2]), int(params[3]));
					break;
					
				case String(Command_ShowWidget):
				case "ShowWidget":
					ShowFollower(int(params[0]));
					break;
					
				case String(Command_HideWidget):
				case "HideWidget":
					HideFollower(int(params[0]));
					break;
			}
		}
		
		// Debug functions
		private function keyDownHandler(e:KeyboardEvent):void {
			switch (e.keyCode) {
				case Keyboard.ENTER:
					trace("enter");
					AddFollower(3, "Random");
					UpdateStats(3, 1.0, 0.2, 3);
					ShowFollower(3);
					break;
				case Keyboard.PAGE_DOWN:
					AddFollower(3, "PgDn");
					UpdateStats(3, 1.0, 0.2, 999);
					ShowFollower(3);
					break;
				case Keyboard.PAGE_UP:
					HideFollower(3);
					break;
				case Keyboard.INSERT:
					AddFollower(2, "Ins");
					UpdateStats(2, 1.0, 0.2, 5000);
					ShowFollower(2);
					break;
				case Keyboard.DELETE:
					HideFollower(2);
					break;
			}
		}
		
		// ---- Begin API Functions ----
		
		public function AddFollower(followerID:int, followerName:String):void {
			//var data:CompanionStatusData = new CompanionStatusData(followerID, followerName);
			
			if (FollowerNameMap[followerName]) {
				followerName = FollowerNameMap[followerName];
			}
			
			var widgetFader:CompanionStatusFader;
			
			widgetFader = FollowerIDToWidgetMap[followerID];
			if (!widgetFader) {
				// Doesn't already exist - create it.
				widgetFader = new CompanionStatusFader();
				widgetFader.widget.data = new CompanionStatusData(followerID, followerName);
				FollowerIDToWidgetMap[followerID] = widgetFader;
				StatusWidgets.push(widgetFader);
				addChild(widgetFader);
			}
			
			widgetFader.widget.data.id = followerID;
			widgetFader.widget.data.name = followerName;
			//widgetFader.widget.data = data;
			
			widgetFader.widget.SetIsDirty();
		}
		
		/*public function RemoveFollower(followerID:int):void {
			HideFollower(followerID, true);
			
			var widget:CompanionStatusWidget = FollowerIDToWidgetMap[followerID];
			StatusWidgets.splice(StatusWidgets.indexOf(widget), 1);
			delete FollowerIDToWidgetMap[followerID];
		}*/
		
		public function UpdateStats(followerID:int, hp:Number, ap:Number, stims:int):void {
			var widget:CompanionStatusWidget = (FollowerIDToWidgetMap[followerID] as CompanionStatusFader).widget;
			if (widget) {
				if (hp < 0) hp = 0;
				if (hp > 1) hp = 1;
				if (ap < 0) ap = 0;
				if (ap > 1) ap = 1;
				widget.data.hp = hp;
				widget.data.ap = ap;
				widget.data.stims = stims;
				widget.SetIsDirty();
			}
		}
		
		public function ShowFollower(followerID:int):void {
			var fader:CompanionStatusFader = FollowerIDToWidgetMap[followerID];
			if (fader) {
				if (!fader.fadeInStarted || fader.fadeOutStarted) {
					fader.ResetFadeState();
					if (ShownStatusWidgets.length > 0) {
						fader.y = GetTargetYForShownIndex(ShownStatusWidgets.length) - Y_SPAWN_OFFSET;
					}
					fader.FadeIn();
					ShownStatusWidgets.push(StatusWidgets.indexOf(fader));
				}
			}
		}
		
		public function HideFollower(followerID:int):void {
			var fader:CompanionStatusFader = FollowerIDToWidgetMap[followerID];
			
			if (fader) {
				fader.FastFadeOut();
				
				/*if (deleteAfterHide) {
					fader.widget.markForDelete = true;
				}*/
			}
		}
		
		// ---- End API Functions ----
		
		// ---- Begin Display / Render Loop Functions ----
		
		private function enterFrameHandler(e:Event):void {
			//trace("enter frame");
			Update();
		}
        
        function Update() {
			var i:int = ShownStatusWidgets.length;
			while (i--) {
				var targetY:Number = this.GetTargetYForShownIndex(i);
				var item:CompanionStatusFader = StatusWidgets[ShownStatusWidgets[i]];
				var deltaY:int = targetY - item.y;
				if (Math.abs(deltaY) > 0) {
					item.y += (deltaY > 0) ? 1 : -1.5;
					
					if (Math.abs(deltaY) < 1) {
						item.y = targetY;
					}
				}
				
				if (item.fadeOutStarted) {
					ShownStatusWidgets.splice(i, 1);
				}
				
				/*if (item.fullyFadedOut) {
					trace("fully faded out");
					trace("Shown widgets:", ShownStatusWidgets.length);
					item.ResetFadeState();
					
					trace("Shown widgets:", ShownStatusWidgets.length);
				}*/
			}
        }
		
		public function GetTargetYForShownIndex(idx:int):Number {
            var targetY:Number = 0;
            for (var i:int=0; i<idx; i++) {
                targetY += this.StatusWidgets[ShownStatusWidgets[i]].height + Y_SPACING;
            }
            return targetY;
        }
		
		// Utility functions
        private function log(str:String):void {
            trace("[" + WIDGET_IDENTIFIER + "] " + str)
        }
	}
}