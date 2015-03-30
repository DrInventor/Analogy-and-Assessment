package ie.nuim.cs.dri.interactROS;

public class ROSInteract {

    static {
      System.loadLibrary("ConsoleApplication2");
   }
 
   private native long GenerateROS(String filename, String longid);
   private native int ROSCountNodes(long neoId);
   private native detailednode[] GetNeo4jGraphNodes(long neoId);
   private native detailednode[] GetNeo4jGraphNodes(String longid);
   
    public static void main(String[] args) {
		//Long lala = new ROSInteract().GenerateROS("test.csv","ABCDEFGHIJ");
		int[] b = new int[3];
		b[0] = 1; b[1] = 2; b[2] = 3;
		detailednode[] nodes = new ROSInteract().GetNeo4jGraphNodes(2);
		System.out.println("b0 " + b[0] + " b1 " + b[1] + " b2 " + b[2]);
		System.out.println("Node stuff " + nodes[0].nodeId + " another : " + nodes[1].nodeId);
		System.out.println("I'm Java : " + nodes.length);
		System.out.println("1 : " + nodes[0].inNodeIds.length);
		System.out.println("2 : " + nodes[0].outNodeIds.length);
		System.out.println("3 : " + nodes[0].Word);
		
    }
    
}
