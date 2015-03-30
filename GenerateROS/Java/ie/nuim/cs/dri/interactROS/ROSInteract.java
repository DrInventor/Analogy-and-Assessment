package ie.nuim.cs.dri.interactROS;

public class ROSInteract {

    static {
      System.loadLibrary("GenerateROS");
   }
 
   private native long GenerateROS(String filename, String longid);
   private native int ROSCountNodes(long neoId);
   private native detailednode[] GetNeo4jGraphNodes(long neoId);
   private native detailednode[] GetNeo4jGraphNodes(String longid);
    
}
