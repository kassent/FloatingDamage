//Created by Action Script Viewer - http://www.buraks.com/asv
package r2k.components
{
    import flash.display.MovieClip;
    import flash.text.*;
    import flash.display.*;
    import flash.events.*;
    import flash.geom.*;
    import flash.net.*;
    import flash.system.*;
    import flash.external.*;
    import adobe.utils.*;
    import flash.accessibility.*;
    import flash.desktop.*;
    import flash.errors.*;
    import flash.filters.*;
    import flash.globalization.*;
    import flash.media.*;
    import flash.net.drm.*;
    import flash.printing.*;
    import flash.profiler.*;
    import flash.sampler.*;
    import flash.sensors.*;
    import flash.text.ime.*;
    import flash.text.engine.*;
    import flash.ui.*;
    import flash.utils.*;
    import flash.xml.*;

    public dynamic class R2K_MeterBarInternal extends MovieClip 
    {

        public function R2K_MeterBarInternal()
        {
            addFrameScript(0, this.frame1, 5, this.frame6, 10, this.frame11, 15, this.frame16, 20, this.frame21, 25, this.frame26, 30, this.frame31, 35, this.frame36, 39, this.frame40);
        }

        function frame1()
        {
            stop();
        }

        function frame6()
        {
            stop();
        }

        function frame11()
        {
            var _local1 = this;
            (_local1["onBarFlashedDark"]());
        }

        function frame16()
        {
            var _local1 = this;
            (_local1["onBarFlashedBright"]());
        }

        function frame21()
        {
            var _local1 = this;
            (_local1["onBarFlashedDark"]());
        }

        function frame26()
        {
            var _local1 = this;
            (_local1["onBarFlashedBright"]());
        }

        function frame31()
        {
            var _local1 = this;
            (_local1["onBarFlashedDark"]());
        }

        function frame36()
        {
            var _local1 = this;
            (_local1["onBarFlashedBright"]());
        }

        function frame40()
        {
            var _local1 = this;
            (_local1["onBarFlashingDone"]());
        }


    }
}//package HUDMenu_fla
