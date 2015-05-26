import ie.nuim.cs.dri.interactROS.*;

public class example2{
	public static void main(String[] args) {
		System.out.println("GenerateROS Example");
		//ROSInteract can be constructed with Neo4j details: location, username, password
		ROSInteract testinteract1 = new ROSInteract("http://localhost:7474/", "neo4j", "drineo4j");
		
		//If it is created with no arguments, then the settings.txt file is used instead
		//ROSInteract testinteract2 = new ROSInteract();
		//It can also be constructed with an additional String for the SQLite db file, if not included it will use current working directory:"dbfile.db" as default
		//ROSInteract("http://localhost:7474/", "neo4j", "drineo4j", "dbfile.db");
		
		//TestNeo4jConnection ensures the username and location are correct; it is not required but returns a simple true/false which may be useful before trying to insert graphs etc.
		if(testinteract1.TestNeo4jConnection()){
			//GenerateROS(filename, longid), if the longid already exists in the database, it will be overwritten
			Long neoid = testinteract1.GenerateROS("Ros1.csv","ABCDEFGHIJ");
			System.out.println("Neo4ID : " + neoid);
			
			//GetNeo4jGraphNodes gets the "detailed" nodes of a particular graph. Arguments can be of the form Neo4ID (int) or longid (String)
			detailednode[] nodes = testinteract1.GetNeo4jGraphNodes(1);
			//If there is an error in reading the graph from the database (e.g. connection error, incorrect id given) then it returns a NULL array
			if(nodes == null)
				System.out.println("NULL ARRAY");
			else{
				System.out.println("0 : " + nodes.length);
				System.out.println("1 : " + nodes[0].inNodeIds.length);	
				System.out.println("2 : " + nodes[0].outNodeIds.length);
				System.out.println("3 : " + nodes[0].Word);
			}
			detailednode[] nodes2 = testinteract1.GetNeo4jGraphNodes("GHJKLERTHY");
			if(nodes2 == null)
				System.out.println("Nodes 2 NULL ARRAY");
			
			//GetNeo4jGraphSimple returns a graph in a "simple" format. Separated lists of all concepts/relations and a list of links in link class format (neo4id or longid accepted)
			simplegraph agraph = testinteract1.GetNeo4jGraphSimple(1);
			if(agraph == null)
				System.out.println("NULL Graph");
			else{
				System.out.println("c : " + agraph.concepts.length + ", r : " + agraph.relations.length + ", h : " + agraph.higherorder.length);
				System.out.println("l : " + agraph.links.length);
				for(int i=0; i< 10 && i < agraph.links.length; ++i){
				System.out.println("Some results : " + agraph.links[i].starttype + ", " + agraph.links[i].endtype + ", " + agraph.links[i].startid + ", " + agraph.links[i].endid);
				}
			}
			
			//GetRawNeo4jGraph returns the raw neo4j json output for a graph (neo4id or longid). empty string is returned if graph doesn't exist or on other error
			String rawneo4j = testinteract1.GetRawNeo4jGraph(1);
			
			//System.out.println(rawneo4j);
		}
	}
}