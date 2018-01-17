package  {
	
	import flash.events.IOErrorEvent;
	import flash.events.TimerEvent;
	import flash.net.URLLoader;
	import flash.net.URLLoaderDataFormat;
	import flash.net.URLRequest;
	import flash.system.ApplicationDomain;
	import flash.text.TextField;
	import flash.utils.Timer;
	import flash.display.MovieClip;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.utils.Dictionary;
	import flash.utils.getDefinitionByName;
	import flash.events.KeyboardEvent;
	import flash.ui.Keyboard;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.display.DisplayObjectContainer;
    import flash.geom.ColorTransform;	
	import Shared.IMenu;
	import FloatingDamage.FloatingDamageFader;
	import FloatingDamage.FloatingDamagePathingSettings;
	
	public class FloatingDamage extends IMenu {
		//elements
		public  var WidgetContainer : MovieClip;
	    public  var BGSCodeObj:Object;
		
		//conditions
		private var isInit: Boolean = false;
		
		//settings
		private var pathingSettings: FloatingDamagePathingSettings;
		private var fWidgetScale: Number = 1.0;
		private var fWidgetOpacity: Number = 1.0;
		
		public function FloatingDamage() 
		{
			log("aaaaaaaaaaaa");
			super();
			this.BGSCodeObj = new Object();
			//Extensions.enabled = true;
			//this.addEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
			log("bbbbbbbbbbbbbbb");
		}
	
		public function onCodeObjCreate() : *
		{
			isInit = true;
			var settings: Object = BGSCodeObj.GetModSettings();
			onModSettingChanged(settings);
		}
      
		public function onCodeObjDestruction() : *
		{
			isInit = false;
			this.BGSCodeObj = null;
		}

		public function onDamageReceived(damage: uint, screenPoint: Array, worldPoint: Array, isBuff: Boolean) : void
		{
			if(this.isInit)
			{
				var fader: FloatingDamageFader = new FloatingDamageFader();
				fader.damage = damage;
				fader.screenData = screenPoint;
				fader.worldData = worldPoint;
				fader.isBuff = isBuff;
				fader.pathingSettings = this.pathingSettings;
				fader.scaleX = fader.scaleY = this.fWidgetScale;
				if(fWidgetOpacity != 1.0)
				{
					fader.alpha = this.fWidgetOpacity;
				}
				WidgetContainer.addChild(fader);				
			}
		}
		
		public function setTransfromColor(color: uint) : void
		{
			if(color >> 24)
			{
				
			}
			else
			{
				this.transform.colorTransform = new ColorTransform(0, 0, 0, 1, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, 0);
			}
		}
		
		public function onModSettingChanged(settings: Object) : void
		{
			if(settings is Object)
			{
				this.fWidgetScale = settings["fWidgetScale"];
				this.fWidgetOpacity = settings["fWidgetOpacity"];
				this.pathingSettings = new FloatingDamagePathingSettings(settings);		
			}
		}
		
        private function log(str:String):void 
		{
            trace("[FloatingDamage]" + str);
        }
	}	
}
