package r2k.components {
	import flash.events.TimerEvent;
	import flash.text.TextField;
	import flash.utils.Timer;
	
	import Shared.AS3.BSUIComponent;
	
	import r2k.components.R2K_MeterBar;
	import flash.display.MovieClip;
	import flash.events.Event;
	import r2k.events.EventWithParams;
	import flash.utils.Dictionary;
	import flash.events.KeyboardEvent;
	import flash.ui.Keyboard;
	
	import hudframework.*;
	
	/**
	* Manages display of multiple companion status widgets.
	*/
	public class CompanionStatusController extends BSUIComponent implements IHUDWidget {
		// Variables
        private static var Y_SPACING:Number = 12;
		private static var Y_SPAWN_OFFSET:Number = 24;
		
		private var hud:HUDFramework;
		
		private var FollowerIDToWidgetMap:Dictionary = new Dictionary(true);
		private var StatusWidgets:Vector.<CompanionStatusFader> = new Vector.<CompanionStatusFader>();
		private var ShownStatusWidgets:Vector.<int> = new Vector.<int>();
		
		public function CompanionStatusController() {
			// Register for commands
			//root.addEventListener("commandUpdate", commandHandler);
			
			var myTimer:Timer = new Timer(3500, 1);
            myTimer.addEventListener(TimerEvent.TIMER, function():void {
				getHUDFramework();
			});
            myTimer.start();
			
			this.addEventListener(Event.ENTER_FRAME, enterFrameHandler);
			stage.addEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
			
			// Test populating data.
			AddFollower(0, "wang");
			UpdateStats(0, 1.0, 0.75, 20);
			ShowFollower(0);
			
			/*AddFollower(1, "KLEO");
			UpdateStats(1, 1.0, 0.75, 20);
			ShowFollower(1);
			
			AddFollower(2, "Piper");
			UpdateStats(2, 1.0, 0.2, 15);
			ShowFollower(2);*/
		}
		
		public function getHUDFramework():void {
			hud = root["HUDFramework"] as HUDFramework;
			if (hud != null) {
				log("HUDFramework is present. Now registered for HUD updates.");
				hud.subscribe(this, "CompanionStatus");
				//(root["HUDFramework"] as MovieClip).addEventListener("HUDUpdate", HUDUpdateHandler);
			} else {
				log("HUDFramework is not present. No updates will be received.");
			}
		}
		
		public function processMessage(command:String, params:Array):void {
			log("Message received.");
			log("Command: " + command + " Params: " + params.toString());
			switch(command) {
				case "SetFollowerName":
					AddFollower(int(params[0]), String(params[1]));
					break;
					
				case "UpdateStats":
					UpdateStats(int(params[0]), Number(params[1]), Number(params[2]), int(params[3]));
					break;
					
				case "ShowWidget":
					ShowFollower(int(params[0]));
					break;
					
				case "HideWidget":
					HideFollower(int(params[0]));
					break;
			}
		}
		
		private function HUDUpdateHandler(e:EventWithParams):void {
			trace("CompanionStatusController: HUD UPDATE RECEIVED.");
			trace(e.params.command);
			
			var cmdArr:Array = e.params.command as Array;
			
			if (cmdArr && cmdArr.length > 0) {
				
				trace("HUDUpdateHandler:", cmdArr[0]);
				switch (cmdArr[0]) {
					case " SetFollowerName ":
						var followerName:String = String(cmdArr[2]);
						AddFollower(int(cmdArr[1]), followerName.substring(1, followerName.length - 1));
						break;
					case " UpdateStats ":
						UpdateStats(int(cmdArr[1]), Number(cmdArr[2]), Number(cmdArr[3]), int(cmdArr[4]));
						break;
					case " ShowWidget ":
						ShowFollower(int(cmdArr[1]));
						break;
					case " HideWidget ":
						HideFollower(int(cmdArr[1]));
						break;
				}
			
			} else {
				trace("HUDUpdateHandler Error - no params received!");
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
			var data:CompanionStatusData = new CompanionStatusData(followerID, followerName);
			var widgetFader:CompanionStatusFader;
			
			widgetFader = FollowerIDToWidgetMap[followerID];
			if (!widgetFader) {
				// Doesn't already exist - create it.
				widgetFader = new CompanionStatusFader();
				FollowerIDToWidgetMap[followerID] = widgetFader;
				StatusWidgets.push(widgetFader);
				addChild(widgetFader);
			}
			
			widgetFader.widget.data = data;
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
            trace("[CompanionStatus] " + str)
        }
	}
}