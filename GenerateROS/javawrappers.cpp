#include "javawrappers.h"

/*
This file is for wrappers between Java and the library. Currently there is only one useful function that processes two csv files (dependency table and POS table).
It requires a LONGID string (the Dr Inventor unique ID?) which is (currently) cross-referenced with a Neo4j SHORTID (>0) in the (possibly generated) SQLITE db file
settings.txt is located in the same folder can contain for Neo4j location and where to store SQLITE db file. If the file does not exist, default settings are used
Default settings : Neo4j "http://localhost:7474" , SQLITE file "data.db"
*/

jlong JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GenerateROS(JNIEnv *env, jobject obj, jstring file, jstring longid){
	//The Neo4j SHORTID is returned to Java, this works the same as -pg
	std::string a1, a3;
	a1 = env->GetStringUTFChars(file, NULL);
	a3 = env->GetStringUTFChars(longid, NULL);
	jlong neoid = ProcessGraphWrap(a1.c_str(), a3.c_str());
	//If Neoid is 0 then it wasn't inserted to the graph.

	return neoid;
}

JNIEXPORT jint JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_ROSCountNodes(JNIEnv *env, jobject obj, jlong neoid){
	struct settings set;
	parsesettings(&set);
	long id = neoid;
	DrInventorSqlitedb sqlite(set.dbfil.c_str());
	int count = sqlite.GetTotalNodeCount(id);
	return count;
}

JNIEXPORT jobjectArray JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphNodes__J(JNIEnv *env, jobject obj, jlong neoid){
	//These functions are given an ID to a graph either in the form of neo4j id or longid(cross-referenced with SQL database)
	//The graph is formed and then the detailed nodes are found - finally an array of java objects is returned (see detailednode.java)
	struct settings set;
	parsesettings(&set);
	GraphProperties *thegraph;
	thegraph = new GraphProperties(neoid, set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
	jobjectArray nodeArray;
	if (thegraph->is_open())
		nodeArray = FillArray(env, thegraph);
	else
		nodeArray = NULL;
	delete thegraph;
	
	return (jobjectArray)nodeArray;
}

JNIEXPORT jobjectArray JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphNodes__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring longid){
	struct settings set;
	parsesettings(&set);
	GraphProperties *thegraph;
	std::string a1;
	a1 = env->GetStringUTFChars(longid, NULL);
	thegraph = new GraphProperties(a1.c_str(), set.dbfil.c_str(), set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str());
	jobjectArray nodeArray;
	if (thegraph->is_open())
		nodeArray = FillArray(env, thegraph);
	else
		nodeArray = NULL;
	delete thegraph;

	return (jobjectArray)nodeArray;
}

jobjectArray FillArray(JNIEnv *env, GraphProperties *thegraph){
	//Fills the array of javaobjects
	thegraph->ConvertGraphtoNodes();
	jclass nodeClass = env->FindClass("ie/nuim/cs/dri/interactROS/detailednode");
	jobjectArray nodeArray = env->NewObjectArray(thegraph->nodes.size(), nodeClass, 0);
	jmethodID constructor = env->GetMethodID(nodeClass, "<init>", "()V");
	if (constructor == NULL){
		printf_s("con error\n");
		return (jobjectArray)NULL;
	}
	if (nodeArray != NULL)
	{
		jfieldID jfnodeid = env->GetFieldID(nodeClass, "nodeId", "J");
		jfieldID jfgraphid = env->GetFieldID(nodeClass, "GraphId", "J");
		jfieldID jfsentenceid = env->GetFieldID(nodeClass, "sentenceId", "J");
		jfieldID jftype = env->GetFieldID(nodeClass, "type", "I");
		jfieldID jfindegree = env->GetFieldID(nodeClass, "inDegree", "I");
		jfieldID jfoutdegree = env->GetFieldID(nodeClass, "outDegree", "I");
		jfieldID jfdegree = env->GetFieldID(nodeClass, "degree", "I");
		jfieldID jfinnodeids = env->GetFieldID(nodeClass, "inNodeIds", "[J");
		jfieldID jfoutnodeids = env->GetFieldID(nodeClass, "outNodeIds", "[J");
		jfieldID jfword = env->GetFieldID(nodeClass, "Word", "Ljava/lang/String;");

		if (jfnodeid == NULL || jfgraphid == NULL || jfsentenceid == NULL || jftype == NULL || jfindegree == NULL || jfoutdegree == NULL || jfdegree == NULL || jfinnodeids == NULL || jfoutnodeids == NULL || jfword == NULL){
			printf_s("I dunno");
			return (jobjectArray)NULL;
		}

		for (unsigned int i = 0; i < thegraph->nodes.size(); ++i){
			jobject tempobj = env->NewObject(nodeClass, constructor);
			env->SetLongField(tempobj, jfnodeid, thegraph->nodes[i].nodeId);
			env->SetLongField(tempobj, jfgraphid, thegraph->nodes[i].GraphId);
			env->SetLongField(tempobj, jfsentenceid, thegraph->nodes[i].sentenceId);
			env->SetIntField(tempobj, jftype, thegraph->nodes[i].type);
			env->SetIntField(tempobj, jfdegree, thegraph->nodes[i].degree);
			env->SetIntField(tempobj, jfindegree, thegraph->nodes[i].inDegree);
			env->SetIntField(tempobj, jfoutdegree, thegraph->nodes[i].outDegree);
			jstring jStr = env->NewStringUTF(thegraph->nodes[i].Word.c_str());
			env->SetObjectField(tempobj, jfword, jStr);
			unsigned int count = thegraph->nodes[i].inNodeIds.size();
			jlongArray arrayinnode = env->NewLongArray(count);
			jlong *temparrayin = new jlong[count];
			for (unsigned int j = 0; j < count; ++j)
				temparrayin[j] = thegraph->nodes[i].inNodeIds[j];
			env->SetLongArrayRegion(arrayinnode, 0, count, temparrayin);
			delete[] temparrayin;
			env->SetObjectField(tempobj, jfinnodeids, arrayinnode);
			count = thegraph->nodes[i].outNodeIds.size();
			jlongArray arrayoutnode = env->NewLongArray(count);
			jlong *temparrayout = new jlong[count];
			for (unsigned int j = 0; j < count; ++j)
				temparrayout[j] = thegraph->nodes[i].outNodeIds[j];
			env->SetLongArrayRegion(arrayoutnode, 0, count, temparrayout);
			delete[] temparrayout;
			env->SetObjectField(tempobj, jfoutnodeids, arrayoutnode);
			env->SetObjectArrayElement(nodeArray, i, tempobj);
		}
	}

	return (jobjectArray)nodeArray;
}

