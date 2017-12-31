//Created by Action Script Viewer - http://www.buraks.com/asv
package r2k.components
{
    import Shared.AS3.BSUIComponent;
    import flash.display.MovieClip;
    import flash.events.Event;

    public dynamic class R2K_MeterBarWidget extends BSUIComponent 
    {

        public var MeterBarInternal_mc:MovieClip;
        private var _startingX:Number;
        private var _startingWidth:Number;
        private var _justification:String;
        private var _percent:Number;
        private var _barAlpha:Number;


        public function get Justification():String
        {
            return (this._justification);
        }

        public function set Justification(_arg1:String)
        {
            if (this._justification != _arg1)
            {
                this._justification = _arg1;
                SetIsDirty();
            };
        }

        public function get Percent():Number
        {
            return (this._percent);
        }

        public function set Percent(_arg1:Number)
        {
            if (this._percent != _arg1)
            {
                this._percent = _arg1;
                SetIsDirty();
            };
        }

        public function get BarAlpha():Number
        {
            return (this._barAlpha);
        }

        public function set BarAlpha(_arg1:Number)
        {
            if (this._barAlpha != _arg1)
            {
                this._barAlpha = _arg1;
                SetIsDirty();
            };
        }

        override public function onAddedToStage():void
        {
            super.onAddedToStage();
            this._startingX = x;
            this._startingWidth = width;
            this.MeterBarInternal_mc.alpha = this.BarAlpha;
			this.addEventListener(Event.ENTER_FRAME, enterFrameHandler);
        }

        override public function redrawUIComponent():void
        {
            super.redrawUIComponent();
            this.MeterBarInternal_mc.alpha = this.BarAlpha;
            //scaleX = this.Percent;
            if (this.Justification == "right")
            {
                x = ((this._startingX + this._startingWidth) - width);
            };
        }
		
		private function enterFrameHandler(e:Event):void {
			var deltaX:Number = this.Percent - scaleX;
			if (Math.abs(deltaX) < 0.01) {
				scaleX = this.Percent;
			} else if (deltaX > 0) {
				// Only animate for increasing health.
				scaleX += 0.01;
			} else {
				scaleX = this.Percent;
			}			
			
			/*if (Math.abs(deltaX) > 0) {
				scaleX += (deltaX > 0) ? 0.01 : -0.01;
				
				if (Math.abs(deltaX) < 0.01) {
					scaleX = this.Percent;
				}
			}*/
		}


    }
}//package 
