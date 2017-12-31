package  {
	
	import flash.text.TextField;
	
	import flash.display.MovieClip;
	
	import flash.events.Event;
	
	public class FloatingDamageWidget extends MovieClip
	{
		// Instances
		public var DisplayText_tf:TextField;
		
		private var _damage: uint = 0;
		
		public function FloatingDamageWidget() 
		{
			//this.DisplayText_tf.text = String(FloatingDamageFader.range(1, 70, true));
		}
		
		public function get damage(): uint 
		{
			return _damage;
		}
		
		public function set damage(val: uint):void 
		{
			_damage = val;
			this.DisplayText_tf.text = String(val);
		}
	}
	
}
