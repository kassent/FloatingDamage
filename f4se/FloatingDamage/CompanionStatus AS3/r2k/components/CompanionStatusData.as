package r2k.components {
    
    public class CompanionStatusData {
        
        public var id:int;
        public var name:String;
        public var hp:Number = 0;
        public var ap:Number = 0;
        public var stims:int;
        
        public function CompanionStatusData(id:int, name:String = "") 
		{
            this.id = id;
            this.name = name;
        }
    }
}