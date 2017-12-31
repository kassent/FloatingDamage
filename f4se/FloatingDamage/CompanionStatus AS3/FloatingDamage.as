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
	import Shared.AS3.BSUIComponent;
	import FloatingDamage.FloatingDamageFader;
	
	public class FloatingDamage extends MovieClip {
		
		private var targetSurface : DisplayObjectContainer;
		private var widgetContainer : MovieClip = new MovieClip();
		private var codeObj: Object;
		private var HUDMenu: MovieClip;
		
		public var buffDamageCounter: uint = 0;
		
		//settings
		//private var canFollow: Boolean = true;
		private var scaleFactor: Number = 1.0;
		private var opacityFactor: Number = 1.0;
		
		public function FloatingDamage() 
		{
			this.addEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
		}
		
		private function addedToStageHandler(e:Event):void 
		{
			this.removeEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
			HUDMenu = stage.getChildAt(0);
			//HUDMenu["FloatingDamageController"] = this;
			codeObj = HUDMenu["f4se"].plugins.FloatingDamage;
			targetSurface = HUDMenu.CenterGroup_mc.HUDCrosshair_mc.CrosshairBase_mc;

			CreateTargetSurface(targetSurface.CrosshairTicks_mc.Up);
			CreateTargetSurface(targetSurface.CrosshairClips_mc);
			HUDMenu.addChild(widgetContainer);
			
			var settings: Object = codeObj.GetModSettings();
			scaleFactor = settings["scale"];
			opacityFactor = settings["opacity"];
			//canFollow = settings["canFollow"];
			
			codeObj.RegisterComponent(widgetContainer);

			log("Init complete...");
		}	

		public function onDamageReceived(damage: uint, screenPoint: Array, worldPoint: Array, isBuff: Boolean) : void
		{
			//var classRef: Class = getDefinitionByName("FloatingDamageFader") as Class;
			var fader: FloatingDamageFader = new FloatingDamageFader();
			fader.damage = damage;
			fader.screenData = screenPoint;
			fader.worldData = worldPoint;
			fader.isBuff = isBuff;
			//fader.canFollow = this.canFollow;
			fader.scaleX = fader.scaleY = this.scaleFactor;
			if(opacityFactor != 1.0)
			{
				fader.alpha = this.opacityFactor;
			}
			widgetContainer.addChild(fader);
		}
		
        private function log(str:String):void 
		{
            trace("[FloatingDamage]" + str)
        }

		public function CreateTargetSurface(targetSurface : MovieClip) : void
		{
			var bgRC = new Sprite();
			bgRC.name = "FloatingDamageSurface";
			bgRC.graphics.beginFill(0x00FF00, 0.0);
			bgRC.graphics.drawRect(0, 0, stage.stageWidth, stage.stageHeight * 1.25);
			bgRC.graphics.endFill();
			
			targetSurface.addChild(bgRC);
			
			var targetSurfaceMatrix:Matrix = targetSurface.transform.concatenatedMatrix.clone();
			// a and d properties of the matrix affects the positioning of pixels along the x and y axis respectively when scaling or rotating an object.
			if (targetSurfaceMatrix.a != 1 || targetSurfaceMatrix.d != 1 || widgetContainer.transform.matrix.a != 1 || widgetContainer.transform.matrix.d != 1) {
				targetSurfaceMatrix.invert();
				bgRC.transform.matrix = targetSurfaceMatrix;
			}
			var globalCoords = targetSurface.globalToLocal(new Point(0, 0));
			bgRC.x = globalCoords.x;
			bgRC.y = globalCoords.y;
		}
	}
	
}
