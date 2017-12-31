package r2k.components {
	
	import flash.display.MovieClip;
	
	
	public class CompanionStatusFader extends HUDFadingListItem {
		public var widget:CompanionStatusWidget;
		
		public function CompanionStatusFader() {
			// constructor code
			widget.fader = this;
		}
	}
	
}
