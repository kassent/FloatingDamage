package  {
	
	import flash.display.MovieClip;
	import flash.events.Event;
	import flash.events.TimerEvent;
	import flash.utils.Timer;
	import flash.text.TextField;
	import flash.utils.getDefinitionByName;
	
	public class ClockWidget extends MovieClip {
		
		private static const REAL_TIME_UPDATE_INTERVAL: int = 1000;
		private static const LOADING_MENU_URL: String = "Interface/LoadingMenu.swf";
		private static const FADER_MENU_URL: String = "Interface/FaderMenu.swf";
		private static const MAIN_MENU_URL: String = "Interface/MainMenu.swf";
		
		private var realTimeTimer: Timer;
		//private var LoadingMenu: MovieClip;
		private var enable24HourClock: Boolean = false;
		
		private var clock: MovieClip;
		
		public function ClockWidget() {
			// constructor code
			super();
			addEventListener(Event.ADDED_TO_STAGE,this.onAddedtoStageHandler);
		}
		
		private function onAddedtoStageHandler(param1:Event) : void
		{
			trace("[ClockWidget] Enter frame...");
			removeEventListener(Event.ADDED_TO_STAGE,this.onAddedtoStageHandler);
			//stage.getChildAt(0)["f4se"].plugins.SW.log(stage.getChildAt(0)["loaderInfo"]["url"] + "::enterFrameHandler");
			try
			{
				enable24HourClock = stage.getChildAt(0)["f4se"].plugins.ClockWidget.GetSettings("iEnable24HourClock");
			}
			catch(err:Error)
			{
				enable24HourClock = false;
				trace("Failed to get settings...");
			}
			switch(stage.getChildAt(0)["loaderInfo"]["url"])
			{
				case LOADING_MENU_URL:
				{
					var Menu: MovieClip = this.parent.parent;
					Menu.removeChild(this.parent);
					//Menu.addChild(this);
					var classRef: Class = getDefinitionByName("Clock") as Class;
					clock = new classRef();
					clock.objectData = {"name": getTimeString()};
					var logo: MovieClip = Menu["VaultTecLogo_mc"];
					logo.addChild(this);
					logo.addChild(clock);
					var firstChild = logo.getChildAt(0);
					firstChild.x -= clock.width;
					clock.x = firstChild.x + firstChild.width / 2;
					clock.y = firstChild.y;
					break;
				}
				case FADER_MENU_URL:
				/*{
					var SpinnerIcon_mc: MovieClip = this.parent.parent;
					var classRef: Class = getDefinitionByName("Clock") as Class;
					clock = new classRef();
					clock.objectData = {"name": getTimeString()};
					SpinnerIcon_mc.addChild(clock);
					var firstChild: MovieClip = SpinnerIcon_mc.getChildAt(0);
					firstChild.x -= clock.width;
					clock.x = firstChild.x + firstChild.width / 2;
					clock.y = firstChild.y;
					break;
				}*/
				case MAIN_MENU_URL:
				{
					var SpinnerIcon_mc: MovieClip = this.parent.parent;
					SpinnerIcon_mc.removeChild(this.parent);
					SpinnerIcon_mc.addChild(this);
					var classRef: Class = getDefinitionByName("Clock") as Class;
					clock = new classRef();
					clock.objectData = {"name": getTimeString()};
					SpinnerIcon_mc.addChild(clock);
					var firstChild: MovieClip = SpinnerIcon_mc.getChildAt(0);
					firstChild.x -= clock.width;
					clock.x = firstChild.x + firstChild.width / 2;
					clock.y = firstChild.y;
					break;
				}
			}
			
			realTimeTimer = new Timer(REAL_TIME_UPDATE_INTERVAL);
			realTimeTimer.addEventListener(TimerEvent.TIMER, onRealTimeTimer);
			realTimeTimer.start();
		}

		private function onRealTimeTimer(e:TimerEvent):void 
		{
			trace("[ClockWidget] On timer trigger...");
			clock.CurrentTime = getTimeString();
		}

		private function getTimeString():String 
		{
			var now:Date = new Date();
			var h:Number = Math.round(now.getHours());
			var m:Number = Math.round(now.getMinutes());
			if(enable24HourClock)
			{
				var str:String = "";
				if (h < 10) str += "0";
				str += h;
				str += ":";
				if (m < 10) str += "0";
				str += m;
			}
			else
			{
				var afternoon:Boolean = (h >= 12) ? true : false;
				if (h > 12) {
					h -= 12;
				}
				var str:String = "";
				if (h < 10) str += "0";
				str += h;
				str += ":";
				if (m < 10) str += "0";
				str += m;
				str += (afternoon ? " PM" : " AM");
			}
			return str;
		}
	}
}
