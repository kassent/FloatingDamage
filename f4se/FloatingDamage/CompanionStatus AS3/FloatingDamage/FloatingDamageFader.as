package{
	
	import flash.display.MovieClip;
	
	import flash.events.Event;

	import flash.geom.Point;
	
	public class FloatingDamageFader extends MovieClip {
		
		public var widget:FloatingDamageWidget;
		public var pathingSettings: FloatingDamagePathingSettings;
		
		private var codeObj: Object;
		
		public var screenData: Array;
		public var worldData: Array;
		public var isBuff: Boolean;
		//public var canFollow: Boolean = true;
		
		private var iHorizontalDirection: int;
		private var fHorizontalSpeed: Number;
		private var fVerticalRisingSpeed: Number;
		private var fVerticalFallDist: Number;
		private var fVerticalRisingDist: Number;

		private var frameCount: uint = 0;	
		private var isFadeOut: Boolean = false;

		public function FloatingDamageFader(dmg: uint, screenPoint: Array, worldPoint: Array, buff: Boolean, showShadow: Boolean, settings: FloatingDamagePathingSettings)
		{
			this.screenData = screenPoint;
			this.worldData = worldPoint;
			this.isBuff = buff;
			this.pathingSettings = settings;
			widget.SetDamageText(dmg, showShadow);
			this.addEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
		}		
		
		private function addedToStageHandler(e:Event):void 
		{
			this.removeEventListener(Event.ADDED_TO_STAGE, addedToStageHandler);
			
			codeObj = stage.getChildAt(0)["Menu_mc"]["BGSCodeObj"];

			fVerticalFallDist = range(pathingSettings.fMinVerticalFallDist, pathingSettings.fMaxVerticalFallDist, false);
			fHorizontalSpeed = range(pathingSettings.fMinHorizontalSpeed, pathingSettings.fMaxHorizontalSpeed, false);

			fVerticalRisingDist = range(pathingSettings.fMinVerticalRisingDist, pathingSettings.fMaxVerticalRisingDist, false);
			fVerticalRisingSpeed = range(pathingSettings.fMinVerticalRisingSpeed, pathingSettings.fMaxVerticalRisingSpeed, false);
			
			iHorizontalDirection = wave;
			
			var screenPos: Point = parent.globalToLocal(new Point(stage.stageWidth * screenData[0], stage.stageHeight * screenData[1]));
			this.x = screenPos.x;
			this.y = screenPos.y;
			
			gotoAndPlay("fadeIn");
			this.addEventListener(Event.ENTER_FRAME, enterFrameHandler);
		}		
		
		
		private function enterFrameHandler(e:Event):void 
		{
			UpdateSelf();
		}
		
		protected function OnFadeOutComplete()
        {
			this.removeEventListener(Event.ENTER_FRAME, enterFrameHandler);
			parent.removeChild(this);
        }
		
		protected function OnFadeInComplete()
        {

        }

		protected function OnFadeOutStart()
        {

        }
		
		private function UpdateSelf(): void
		{
			++frameCount;
			var newScreenPosArr : Array = null;
			var newScreenPos : Point = null;
			var offsetY : Number = 0;
			if (isBuff)
			{
				newScreenPosArr = codeObj.WorldtoScreen(worldData[0], worldData[1], worldData[2]);
				newScreenPos = parent.globalToLocal(new Point(stage.stageWidth * newScreenPosArr[0], stage.stageHeight * newScreenPosArr[1]));
				this.x = newScreenPos.x;
				offsetY = pathingSettings.fEffectDamageRisingSpeed * frameCount;

				if (!isFadeOut && (offsetY > fVerticalRisingDist || frameCount >= 300))
				{
					isFadeOut = true;
					gotoAndPlay("fadeOut");
				}
				this.y = newScreenPos.y - offsetY;
			}
			else
			{
				newScreenPosArr = codeObj.WorldtoScreen(worldData[0], worldData[1], worldData[2]);
				newScreenPos = parent.globalToLocal(new Point(stage.stageWidth * newScreenPosArr[0], stage.stageHeight * newScreenPosArr[1]));
				this.x = newScreenPos.x + fHorizontalSpeed * frameCount * iHorizontalDirection;
				offsetY = pathingSettings.fGravitationalConstant * frameCount * frameCount - fVerticalRisingSpeed * frameCount;
				if (!isFadeOut && (offsetY > fVerticalFallDist || frameCount >= 300))
				{
					isFadeOut = true;
					gotoAndPlay("fadeOut");
				}
				this.y = newScreenPos.y + offsetY;
			}
		}
		
		public static function get boolean():Boolean
		{
			return Math.random() < 0.5;
		}
		
		public static function get wave():int
		{
			return boolean ? 1 : -1;
		}

		public static function range(num1:Number,num2:Number,isInt:Boolean = true):Number
		{
			var num:Number = Math.random() * Number(num2 - num1) + num1;
			if (isInt)
			{
				num = Math.floor(num);
			}
			return num;
		}
		
        private function log(str:String):void 
		{
            trace("[FloatingDamage]" + str);
        }
	}	
}
