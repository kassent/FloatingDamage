package  {
	
	import flash.text.TextField;
	
	import flash.display.MovieClip;
	
	import flash.events.Event;
	
	import flash.filters.DropShadowFilter;
	
	import Shared.GlobalFunc;
	
	public class FloatingDamageWidget extends MovieClip
	{
		// Instances
		public var DisplayText_tf:TextField;
		
		public function FloatingDamageWidget() 
		{
			//this.DisplayText_tf.text = String(FloatingDamageFader.range(1, 70, true));
			//var shadow:DropShadowFilter = new DropShadowFilter(0.3, 45, 0, 0.6, 1, 1, 10, 1, false, false);
			//var filtersArray:Array = new Array(shadow);
			//DisplayText_tf.filters = filtersArray;
		}
		
		public function SetDamageText(dmg: uint, showShadow: Boolean) : void
		{
			if(showShadow)
			{
				this.DisplayText_tf.text = String(FloatingDamageFader.range(1, 70, true));
				var shadow:DropShadowFilter = new DropShadowFilter(0.3, 45, 0, 0.6, 1, 1, 10, 1, false, false);
				var filtersArray:Array = new Array(shadow);
				DisplayText_tf.filters = filtersArray;			
			}
			GlobalFunc.SetText(this.DisplayText_tf, String(dmg),false);
		}
	}
	
}
