//Created by Action Script Viewer - http://www.buraks.com/asv
package Shared
{
    import flash.events.Event;
    import flash.display.MovieClip;

    public class PlatformRequestEvent extends Event 
    {

        public static const PLATFORM_REQUEST:String = "GetPlatform";

        var _target:MovieClip;

        public function PlatformRequestEvent(_arg1:MovieClip)
        {
            super(PLATFORM_REQUEST);
            this._target = _arg1;
        }

        public function RespondToRequest(_arg1:uint, _arg2:Boolean)
        {
            this._target.SetPlatform(_arg1, _arg2);
        }


    }
}//package Shared
