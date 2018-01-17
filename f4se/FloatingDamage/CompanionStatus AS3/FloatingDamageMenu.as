package
{
	import Shared.GlobalFunc;
	import Shared.IMenu;
	import flash.display.MovieClip;
	import flash.events.Event;
    import flash.geom.ColorTransform;	
   
	public class FloatingDamageMenu extends IMenu
	{
		//public  var FloatingDamageSurface : MovieClip;
	    public  var BGSCodeObj:Object;
		
		public	var	showShadowEffect: Boolean = false;
  
		//conditions
		private var isInit: Boolean = false;
		
		//settings
		private var pathingSettings: FloatingDamagePathingSettings;
		private var fWidgetScale: Number = 1.0;
		private var fWidgetOpacity: Number = 1.0;
	   
		public function FloatingDamageMenu()
		{
			super();
			this.BGSCodeObj = new Object();

		}
		public function onCodeObjCreate() : *
		{
			var settings: Object = BGSCodeObj.GetModSettings();
			onModSettingChanged(settings);
			isInit = true;
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
				var fader: FloatingDamageFader = new FloatingDamageFader(damage, screenPoint, worldPoint, isBuff, showShadowEffect, this.pathingSettings);
				fader.scaleX = fader.scaleY = this.fWidgetScale;
				if(fWidgetOpacity != 1.0)
				{
					fader.alpha = this.fWidgetOpacity;
				}
				this.addChild(fader);				
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
		
		public function ProcessUserEvent(param1:String, param2:Boolean) : *
		{

		}
	}
}
