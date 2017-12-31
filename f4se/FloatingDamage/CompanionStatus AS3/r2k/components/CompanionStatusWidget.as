package r2k.components {
	import flash.text.TextField;
	
	import Shared.AS3.BSUIComponent;
	
	import r2k.components.R2K_MeterBar;
	import flash.display.MovieClip;
	
	public class CompanionStatusWidget extends BSUIComponent {
		// Instances
		public var DisplayText_tf:TextField;
		public var HPBar_mc:R2K_MeterBar;
		public var APBar_mc:R2K_MeterBar;
		public var Stims_tf:TextField;
		
		// Variables
		private var _data:CompanionStatusData;
		private var _fader:CompanionStatusFader;
		
		// If true, will be deleted by the manager after this widget has faded out.
		public var markForDelete:Boolean = false;
		
		public var shown:Boolean = false;
		
		public function CompanionStatusWidget() {
			this.bShowBrackets = true;
			ShadedBackgroundMethod = "Shader";
            ShadedBackgroundType = "normal";
			bUseShadedBackground = true;
			this.BracketStyle = "vertical";
			this.bracketPaddingY = 0;		// default 0
			this.bracketCornerLength = 3;	// default 6
			setProperties();
			populateFields();
		}
		
		public function get data():CompanionStatusData {
			return _data;
		}
		public function set data(val:CompanionStatusData):void {
			_data = val;
			populateFields();
		}
		
		public function get fader():CompanionStatusFader {
			return _fader;
		}
		public function set fader(val:CompanionStatusFader):void {
			_fader = val;
		}
		
		private function setProperties():void {
			// HPBar
			
			try {
                this.HPBar_mc["componentInspectorSetting"] = true;
            } catch(e:Error) {}
            this.HPBar_mc.BarAlpha = 1;
            this.HPBar_mc.bracketCornerLength = 6;
            this.HPBar_mc.bracketLineWidth = 1.5;
            this.HPBar_mc.bracketPaddingX = 0;
            this.HPBar_mc.bracketPaddingY = 0;
            this.HPBar_mc.BracketStyle = "horizontal";
            this.HPBar_mc.bShowBrackets = false;
            this.HPBar_mc.bUseShadedBackground = false;
            this.HPBar_mc.Justification = "left";
            this.HPBar_mc.Percent = 0.5;
            this.HPBar_mc.ShadedBackgroundMethod = "Shader";
            this.HPBar_mc.ShadedBackgroundType = "normal";
            try {
                this.HPBar_mc["componentInspectorSetting"] = false;
            } catch(e:Error) {}
			
			// APBar
			
			try {
                this.APBar_mc["componentInspectorSetting"] = true;
            } catch(e:Error) {}
            this.APBar_mc.BarAlpha = 1;
            this.APBar_mc.bracketCornerLength = 6;
            this.APBar_mc.bracketLineWidth = 1.5;
            this.APBar_mc.bracketPaddingX = 0;
            this.APBar_mc.bracketPaddingY = 0;
            this.APBar_mc.BracketStyle = "horizontal";
            this.APBar_mc.bShowBrackets = false;
            this.APBar_mc.bUseShadedBackground = false;
            this.APBar_mc.Justification = "left";
            this.APBar_mc.Percent = 0.2;
            this.APBar_mc.ShadedBackgroundMethod = "Shader";
            this.APBar_mc.ShadedBackgroundType = "normal";
            try {
                this.APBar_mc["componentInspectorSetting"] = false;
            } catch(e:Error) {}
		}
		
		// Populates fields from data
		private function populateFields():void {
			if (_data) {
				this.DisplayText_tf.text = _data.name;
			}
		}
		
		// Display
		override public function redrawUIComponent():void {
			if (_data) {
				this.DisplayText_tf.text = _data.name;
				this.HPBar_mc.Percent = _data.hp;
				this.APBar_mc.Percent = _data.ap;
				this.Stims_tf.text = String(_data.stims);
			} else {
				//trace("WIDGET - no data available.");
			}
		}
	}
}