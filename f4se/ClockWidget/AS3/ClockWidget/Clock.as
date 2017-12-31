package
{
	import flash.display.MovieClip;
	import flash.text.TextField;
	import flash.events.Event;
	
	public class Clock extends MovieClip
	{
		public var DisplayText_tf:TextField;
		public var objectData: Object = null;
		
		public function Clock()
		{
			addEventListener(Event.ADDED_TO_STAGE,onAddedToStage); 
		}
		
		public function onAddedToStage(evt:Event)
		{
			if(objectData) {
				DisplayText_tf.autoSize = "left";
				CurrentTime = objectData.name;
			}
		}
		
		public function set CurrentTime(a_name: String)
		{
			DisplayText_tf.text = a_name;
		}
	}
}
