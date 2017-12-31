//Created by Action Script Viewer - http://www.buraks.com/asv
package Shared.AS3
{
    import flash.display.MovieClip;
    import flash.geom.Point;
    import flash.geom.Rectangle;
    import flash.display.CapsStyle;
    import flash.display.JointStyle;

    public dynamic class BSBracketClip extends MovieClip 
    {

        static const BR_HORIZONTAL:String = "horizontal";
        static const BR_VERTICAL:String = "vertical";
        static const BR_CORNERS:String = "corners";
        static const BR_FULL:String = "full";

        private var _drawPos:Point;
        private var _clipRect:Rectangle;
        private var _lineThickness:Number;
        private var _cornerLength:Number;
        private var _padding:Point;
        private var _style:String;


        public function BracketPair()
        {
        }

        public function ClearBrackets()
        {
            graphics.clear();
        }

        public function redrawUIComponent(_arg1:BSUIComponent, _arg2:Number, _arg3:Number, _arg4:Point, _arg5:String)
        {
            this._clipRect = _arg1.getBounds(_arg1);
            this._lineThickness = _arg2;
            this._cornerLength = _arg3;
            this._padding = _arg4;
            this._clipRect.inflatePoint(this._padding);
            this._style = _arg5;
            this.ClearBrackets();
            graphics.lineStyle(this._lineThickness, 0xFFFFFF, 1, false, "normal", CapsStyle.SQUARE, JointStyle.MITER, 3);
            switch (this._style)
            {
                case BR_HORIZONTAL:
                    this.doHorizontal();
                    return;
                case BR_VERTICAL:
                    this.doVertical();
                    return;
                case BR_CORNERS:
                    this.doCorners();
                    return;
                case BR_FULL:
                    this.doFull();
                    return;
            };
        }

        private function doHorizontal()
        {
            this._drawPos = new Point((this._clipRect.left + this._cornerLength), this._clipRect.top);
            this.moveTo();
            this.LineX(this._clipRect.left);
            this.LineY(this._clipRect.bottom);
            this.LineX((this._clipRect.left + this._cornerLength));
            this.MoveX((this._clipRect.right - this._cornerLength));
            this.LineX(this._clipRect.right);
            this.LineY(this._clipRect.top);
            this.LineX((this._clipRect.right - this._cornerLength));
        }

        private function doVertical()
        {
            this._drawPos = new Point(this._clipRect.left, (this._clipRect.top + this._cornerLength));
            this.moveTo();
            this.LineY(this._clipRect.top);
            this.LineX(this._clipRect.right);
            this.LineY((this._clipRect.top + this._cornerLength));
            this.MoveY((this._clipRect.bottom - this._cornerLength));
            this.LineY(this._clipRect.bottom);
            this.LineX(this._clipRect.left);
            this.LineY((this._clipRect.bottom - this._cornerLength));
        }

        private function doCorners()
        {
            this._drawPos = new Point((this._clipRect.left + this._cornerLength), this._clipRect.top);
            this.moveTo();
            this.LineX(this._clipRect.left);
            this.LineY((this._clipRect.top + this._cornerLength));
            this.MoveY((this._clipRect.bottom - this._cornerLength));
            this.LineY(this._clipRect.bottom);
            this.LineX((this._clipRect.left + this._cornerLength));
            this.MoveX((this._clipRect.right - this._cornerLength));
            this.LineX(this._clipRect.right);
            this.LineY((this._clipRect.bottom - this._cornerLength));
            this.MoveY((this._clipRect.top + this._cornerLength));
            this.LineY(this._clipRect.top);
            this.LineX((this._clipRect.right - this._cornerLength));
        }

        private function doFull()
        {
            this._drawPos = new Point(this._clipRect.left, this._clipRect.top);
            this.moveTo();
            this.LineY(this._clipRect.bottom);
            this.LineX(this._clipRect.right);
            this.LineY(this._clipRect.top);
            this.LineX(this._clipRect.left);
        }

        private function LineX(_arg1:Number)
        {
            this._drawPos.x = _arg1;
            this.lineTo();
        }

        private function LineY(_arg1:Number)
        {
            this._drawPos.y = _arg1;
            this.lineTo();
        }

        private function MoveX(_arg1:Number)
        {
            this._drawPos.x = _arg1;
            this.moveTo();
        }

        private function MoveY(_arg1:Number)
        {
            this._drawPos.y = _arg1;
            this.moveTo();
        }

        private function lineTo()
        {
            graphics.lineTo(this._drawPos.x, this._drawPos.y);
        }

        private function moveTo()
        {
            graphics.moveTo(this._drawPos.x, this._drawPos.y);
        }


    }
}//package Shared.AS3
