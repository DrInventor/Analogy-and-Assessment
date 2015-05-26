package ie.nuim.cs.dri.interactROS;

public class simplegraph{
	public String concepts[], relations[], higherorder[];
	
	public class link{
		public char starttype, endtype;
		public long startid, endid;
	}
	
	public link[] links;
}