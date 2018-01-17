package {
    
    public class FloatingDamagePathingSettings {

		public var fMinHorizontalSpeed: Number = 2.5;
		public var fMaxHorizontalSpeed: Number = 4.0;
		public var fMinVerticalRisingSpeed: Number = 1.5;
		public var fMaxVerticalRisingSpeed: Number = 4.5;
		public var fGravitationalConstant: Number = 0.08;
		public var fEffectDamageRisingSpeed: Number = 1.5;	
		public var fMaxVerticalRisingDist: Number = 105.0;
		public var fMinVerticalRisingDist: Number = 90.0;
		public var fMaxVerticalFallDist: Number = 120.0;
		public var fMinVerticalFallDist: Number = 80.0;

        public function FloatingDamagePathingSettings(settings: Object) 
		{
			this.fMinHorizontalSpeed = settings["fMinHorizontalSpeed"];
			this.fMaxHorizontalSpeed = settings["fMaxHorizontalSpeed"];
			this.fMinVerticalRisingSpeed = settings["fMinVerticalRisingSpeed"];
			this.fMaxVerticalRisingSpeed = settings["fMaxVerticalRisingSpeed"];
			this.fGravitationalConstant = settings["fGravitationalConstant"];
			this.fEffectDamageRisingSpeed = settings["fEffectDamageRisingSpeed"];	
			this.fMaxVerticalRisingDist = settings["fMaxVerticalRisingDist"];	
			this.fMinVerticalRisingDist = settings["fMinVerticalRisingDist"];	
			this.fMaxVerticalFallDist = settings["fMaxVerticalFallDist"];	
			this.fMinVerticalFallDist = settings["fMinVerticalFallDist"];				
        }
    }
}
