package ie.nuim.cs.dri.interactROS;

public class ROSInteract {

    static {
      System.loadLibrary("GenerateROS");
   }
 
   public native long GenerateROS(String filename, String longid);
   public native int ROSCountNodes(long neoId);
   public native detailednode[] GetNeo4jGraphNodes(long neoId);
   public native detailednode[] GetNeo4jGraphNodes(String longid);
   public native simplegraph GetNeo4jGraphSimple(long neoId);
   public native simplegraph GetNeo4jGraphSimple(String longid);
   public native String GetRawNeo4jGraph(long neoId);
   public native String GetRawNeo4jGraph(String longid);
   public native boolean TestNeo4jConnection();
   private native boolean CheckDllVersion(double expected);
   
   	private boolean testneo4j, versionok;
	private String neo4loc, neo4user, neo4pass, sqdbfile;
	
	public void DoTestingofNeo4j(boolean dotesting){
		testneo4j = dotesting;
	}
	
	public ROSInteract(){
		this("","","");
	}
	
	public ROSInteract(String location, String username, String password){
		this(location,username,password,"data.db");
	}
	
	public ROSInteract(String location, String username, String password, String dbfile){
		neo4loc = location;
		neo4user = username;
		neo4pass = password;
		sqdbfile = dbfile;
		testneo4j = true;
		versionok = CheckDllVersion(1);
	}
	
	public boolean LibraryVersionMatch(){
		return versionok;
	}
   
}
