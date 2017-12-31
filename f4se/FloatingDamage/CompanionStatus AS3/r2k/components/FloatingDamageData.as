package {
    
    public class FloatingDamageData {
        
		public var damage: uint = 15;
		public var screenData: Array;
		public var worldData: Array;
		/*
		public var xScreenPercent: Number;
		public var yScreenPercent: Number;
		public var zScreenPercent: Number;
		public var xWorldCoord: Number;
		public var yWorldCoord: Number;
		public var zWorldCoord: Number;
		*/
        public function FloatingDamageData(damageData: uint, screenArr: Array, worldArr: Array) 
		{
			this.damage = damageData;
			this.screenData = screenArr;
			this.worldData = worldArr;
        }
    }
}