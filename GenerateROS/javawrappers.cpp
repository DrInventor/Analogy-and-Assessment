#include "javawrappers.h"

#define dllversion 1

/*
This file is for wrappers between Java and the library. Currently there is only one useful function that processes two csv files (dependency table and POS table).
It requires a LONGID string (the Dr Inventor unique ID?) which is (currently) cross-referenced with a Neo4j SHORTID (>0) in the (possibly generated) SQLITE db file
settings.txt is located in the same folder can contain for Neo4j location and where to store SQLITE db file. If the file does not exist, default settings are used
Default settings : Neo4j "http://localhost:7474" , SQLITE file "data.db"
*/
JNIEXPORT jboolean JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_CheckDllVersion(JNIEnv *, jobject, jdouble testing){
	if (testing == dllversion){
		return true;
	}
	else{
		printf_s("Mismatch in DLL and JAR version number. This error can be ignored but issues may occur.\n");
		return false;
	}
}

JNIEXPORT jboolean JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_TestNeo4jConnection(JNIEnv *env, jobject obj){
	//Tests that the neo4j database is working. If it is, sets the testneo4j variable in the java object to false , i.e. no longer needs to test
	struct settings set;
	Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);
	Neo4jInteract neotest(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), true);
	jboolean toreturn = neotest.isopen();
	jclass TestClass = env->GetObjectClass(obj);
	jfieldID theboolean = env->GetFieldID(TestClass, "testneo4j", "Z");
	env->SetBooleanField(obj, theboolean, !toreturn);

	return toreturn;
}

jlong JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GenerateROS(JNIEnv *env, jobject obj, jstring file, jstring longid){
	//The Neo4j SHORTID is returned to Java, this works the same as -pg
	std::string a1, a3;
	a1 = env->GetStringUTFChars(file, NULL);
	a3 = env->GetStringUTFChars(longid, NULL);
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);
	jlong neoid = ProcessGraphWrap(a1.c_str(), a3.c_str(), needtotestneo4j, &set);
	//If Neoid is 0 then it wasn't inserted to the graph.
	if (neoid != 0){
		jboolean toreturn = false;
		jclass TestClass = env->GetObjectClass(obj);
		jfieldID theboolean = env->GetFieldID(TestClass, "testneo4j", "Z");
		env->SetBooleanField(obj, theboolean, toreturn);
	}
	return neoid;
}

JNIEXPORT jint JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_ROSCountNodes(JNIEnv *env, jobject obj, jlong neoid){
	struct settings set;
	Java_ReadDataFromObject(env, obj, &set);
	if(set.neo4j.length() == 0) parsesettings(&set);
	long id = neoid;
	DrInventorSqlitedb sqlite(set.dbfil.c_str());
	int count = sqlite.GetTotalNodeCount(id);
	return count;
}

JNIEXPORT jobjectArray JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphNodes__J(JNIEnv *env, jobject obj, jlong neoid){
	//These functions are given an ID to a graph either in the form of neo4j id or longid(cross-referenced with SQL database)
	//The graph is formed and then the detailed nodes are found - finally an array of java objects is returned (see detailednode.java)
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);
	GraphProperties *thegraph;
	long theid = (long)neoid;
	thegraph = new GraphProperties(theid, set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), needtotestneo4j);
	jobjectArray nodeArray;
	if (thegraph->is_open()){
		nodeArray = FillArray(env, thegraph);
		if (needtotestneo4j){
			jboolean toreturn = false;
			jclass TestClass = env->GetObjectClass(obj);
			jfieldID theboolean = env->GetFieldID(TestClass, "testneo4j", "Z");
			env->SetBooleanField(obj, theboolean, toreturn);
		}
	}
	else
		nodeArray = NULL;
	delete thegraph;
	
	return (jobjectArray)nodeArray;
}

JNIEXPORT jobjectArray JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphNodes__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring longid){
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);
	GraphProperties *thegraph;
	std::string a1;
	a1 = env->GetStringUTFChars(longid, NULL);
	thegraph = new GraphProperties(a1.c_str(), set.dbfil.c_str(), set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), needtotestneo4j);
	jobjectArray nodeArray;
	if (thegraph->is_open()){
		nodeArray = FillArray(env, thegraph);
		if (needtotestneo4j){
			jboolean toreturn = false;
			jclass TestClass = env->GetObjectClass(obj);
			jfieldID theboolean = env->GetFieldID(TestClass, "testneo4j", "Z");
			env->SetBooleanField(obj, theboolean, toreturn);
		}
	}
	else
		nodeArray = NULL;
	delete thegraph;

	return (jobjectArray)nodeArray;
}

JNIEXPORT jobject JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphSimple__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring longid){
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);

	DrInventorSqlitedb *sqlitedb;
	sqlitedb = new DrInventorSqlitedb(set.dbfil.c_str());
	long neo4id;
	if (sqlitedb->isopen()){
		std::string a1;
		a1 = env->GetStringUTFChars(longid, NULL);
		neo4id = sqlitedb->GetIDfromLongID(a1.c_str());
		delete sqlitedb;
		if (neo4id == -1)
			return NULL;
	}
	else{
		delete sqlitedb;
		return NULL;
	}
	return Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphSimple__J(env, obj, neo4id);
}

JNIEXPORT jobject JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetNeo4jGraphSimple__J(JNIEnv *env, jobject obj, jlong neoid){
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);

	GraphProperties *thegraph;
	thegraph = new GraphProperties(neoid, set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), needtotestneo4j);
	jobject thesimplegraph;
	if (thegraph->is_open()){
		
		jclass simplegraphClass = env->FindClass("ie/nuim/cs/dri/interactROS/simplegraph");
		jmethodID constructor = env->GetMethodID(simplegraphClass, "<init>", "()V");
		if (constructor == NULL){
			printf_s("con error\n");
			delete thegraph;
			return NULL;
		}
		thesimplegraph = env->NewObject(simplegraphClass, constructor);
		if (thesimplegraph == NULL){
			printf_s("jobject error\n");
			delete thegraph;
			return NULL;
		}
		
		jfieldID jfconcepts = env->GetFieldID(simplegraphClass, "concepts", "[Ljava/lang/String;");
		jfieldID jfrelations = env->GetFieldID(simplegraphClass, "relations", "[Ljava/lang/String;");
		jfieldID jfhighorder = env->GetFieldID(simplegraphClass, "higherorder", "[Ljava/lang/String;");

		jfieldID jflinks = env->GetFieldID(simplegraphClass, "links", "[Lie/nuim/cs/dri/interactROS/simplegraph$link;");
		if (jfconcepts == NULL || jfrelations == NULL || jfhighorder == NULL){
			printf_s("error with field and strings\n");
			delete thegraph;
			return NULL;
		}

		unsigned int count = (unsigned int)thegraph->thegraph.concepts.size();
		jclass stringClass = env->FindClass("Ljava/lang/String;");
		jobjectArray conceptsarray = env->NewObjectArray(count, stringClass, NULL);
		if (conceptsarray == NULL){
			printf_s("error with conceptsarray\n");
			delete thegraph;
			return NULL;
		}
		for (unsigned int i = 0; i < count; ++i){
			env->SetObjectArrayElement(conceptsarray, i, env->NewStringUTF(thegraph->thegraph.concepts[i].word.c_str()));
		}
		env->SetObjectField(thesimplegraph, jfconcepts, conceptsarray);

		count = (unsigned int)thegraph->thegraph.relations.size();
		jobjectArray relationsarray = env->NewObjectArray(count, stringClass, NULL);
		for (unsigned int i = 0; i < count; ++i){
			env->SetObjectArrayElement(relationsarray, i, env->NewStringUTF(thegraph->thegraph.relations[i].word.c_str()));
		}
		
		env->SetObjectField(thesimplegraph, jfrelations, relationsarray);
		count = 0;
		jobjectArray higherarray = env->NewObjectArray(count, stringClass, NULL);
		env->SetObjectField(thesimplegraph, jfhighorder, higherarray);

		jclass linksClass = env->FindClass("ie/nuim/cs/dri/interactROS/simplegraph$link");
		if (linksClass == NULL){
			printf_s("error linksclass\n");
			return NULL;
		}

		count = 2 * (unsigned int)thegraph->thegraph.links.size();
		jobjectArray linksarray = env->NewObjectArray(count, linksClass, NULL);
		if (linksarray == NULL){
			printf_s("error with linksarray\n");
			return NULL;
		}

		jmethodID constructorlinks = env->GetMethodID(linksClass, "<init>", "(Lie/nuim/cs/dri/interactROS/simplegraph;)V");
		jfieldID jfstartc = env->GetFieldID(linksClass, "starttype", "C");
		jfieldID jfendc = env->GetFieldID(linksClass, "endtype", "C");
		jfieldID jfstartn = env->GetFieldID(linksClass, "startid", "J");
		jfieldID jfendn = env->GetFieldID(linksClass, "endid", "J");
		for (unsigned int i = 0; i < count/2; ++i){
			jobject tempobj1 = env->NewObject(linksClass, constructorlinks);
			env->SetCharField(tempobj1, jfstartc, 'c');
			env->SetLongField(tempobj1, jfstartn, thegraph->thegraph.links[i].sbj);
			env->SetCharField(tempobj1, jfendc, 'r');
			env->SetLongField(tempobj1, jfendn, thegraph->thegraph.links[i].vb);
			env->SetObjectArrayElement(linksarray, 2 * i, tempobj1);
			jobject tempobj2 = env->NewObject(linksClass, constructorlinks);
			env->SetCharField(tempobj2, jfstartc, 'r');
			env->SetLongField(tempobj2, jfstartn, thegraph->thegraph.links[i].vb);
			env->SetCharField(tempobj2, jfendc, 'c');
			env->SetLongField(tempobj2, jfendn, thegraph->thegraph.links[i].obj);
			env->SetObjectArrayElement(linksarray, (2 * i) + 1, tempobj2);
		}
		env->SetObjectField(thesimplegraph, jflinks, linksarray);
	}
	else
		thesimplegraph = NULL;
	delete thegraph;

	return thesimplegraph;
}

JNIEXPORT jstring JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetRawNeo4jGraph__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring longid){
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);

	DrInventorSqlitedb *sqlitedb;
	sqlitedb = new DrInventorSqlitedb(set.dbfil.c_str());
	long neoid=-1;
	if (sqlitedb->isopen()){
		std::string a1;
		a1 = env->GetStringUTFChars(longid, NULL);
		neoid = sqlitedb->GetIDfromLongID(a1.c_str());
		delete sqlitedb;
	}
	else{
		delete sqlitedb;
	}
	std::string response = "";
	if (neoid != -1){
		Neo4jInteract *neodb;
		neodb = new Neo4jInteract(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), needtotestneo4j);
		std::string response = "";
		if (neodb->isopen()){
			std::stringstream tmpcmd;
			std::vector<std::string> commands;
			tmpcmd << "match (n:Noun) where n.Graphid=" << neoid << " return count(n); ";
			commands.push_back(tmpcmd.str());
			tmpcmd.str("");
			tmpcmd << "match (n:Noun) where n.Graphid=" << neoid << " return n.Word, n.GVLabel, id(n);";
			commands.push_back(tmpcmd.str());
			tmpcmd.str("");
			tmpcmd << "match (n:Verb) where n.Graphid=" << neoid << " return count(n);";
			commands.push_back(tmpcmd.str());
			tmpcmd.str("");
			tmpcmd << "match (n:Verb) where n.Graphid=" << neoid << " return n.Word, n.GVLabel, n.SentID, id(n);";
			commands.push_back(tmpcmd.str());
			tmpcmd.str("");
			tmpcmd << "match (n:Noun)-[r:sbj]-(m:Verb)-[p:obj]-(o:Noun) where n.Graphid=" << neoid << " return n.GVLabel,m.GVLabel,o.GVLabel;";
			commands.push_back(tmpcmd.str());
			tmpcmd.str("");
			neodb->neo4cyphermultiple(&commands, &response);
		}
		delete neodb;
	}
	jstring result = env->NewStringUTF(response.c_str());
	return result;
}

JNIEXPORT jstring JNICALL Java_ie_nuim_cs_dri_interactROS_ROSInteract_GetRawNeo4jGraph__J(JNIEnv *env, jobject obj, jlong neoid){
	struct settings set;
	bool needtotestneo4j = Java_ReadDataFromObject(env, obj, &set);
	if (set.neo4j.length() == 0) parsesettings(&set);
	Neo4jInteract *neodb;
	neodb = new Neo4jInteract(set.neo4j.c_str(), set.neouser.c_str(), set.neopass.c_str(), needtotestneo4j);
	std::string response = "";
	if (neodb->isopen()){
		std::stringstream tmpcmd;
		std::vector<std::string> commands;
		tmpcmd << "match (n:Noun) where n.Graphid=" << neoid << " return count(n); ";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Noun) where n.Graphid=" << neoid << " return n.Word, n.GVLabel, id(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Verb) where n.Graphid=" << neoid << " return count(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Verb) where n.Graphid=" << neoid << " return n.Word, n.GVLabel, n.SentID, id(n);";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		tmpcmd << "match (n:Noun)-[r:sbj]-(m:Verb)-[p:obj]-(o:Noun) where n.Graphid=" << neoid << " return n.GVLabel,m.GVLabel,o.GVLabel;";
		commands.push_back(tmpcmd.str());
		tmpcmd.str("");
		neodb->neo4cyphermultiple(&commands, &response);
	}
	delete neodb;
	jstring result = env->NewStringUTF(response.c_str());
	return result;
}


jobjectArray FillArray(JNIEnv *env, GraphProperties *thegraph){
	//Fills the array of javaobjects
	thegraph->ConvertGraphtoNodes();
	jclass nodeClass = env->FindClass("ie/nuim/cs/dri/interactROS/detailednode");
	jobjectArray nodeArray = env->NewObjectArray((jsize)thegraph->nodes.size(), nodeClass, 0);
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
			unsigned int count = (unsigned int)thegraph->nodes[i].inNodeIds.size();
			jlongArray arrayinnode = env->NewLongArray(count);
			jlong *temparrayin = new jlong[count];
			for (unsigned int j = 0; j < count; ++j)
				temparrayin[j] = thegraph->nodes[i].inNodeIds[j];
			env->SetLongArrayRegion(arrayinnode, 0, count, temparrayin);
			delete[] temparrayin;
			env->SetObjectField(tempobj, jfinnodeids, arrayinnode);
			count = (unsigned int)thegraph->nodes[i].outNodeIds.size();
			jlongArray arrayoutnode = env->NewLongArray(count);
			jlong *temparrayout = new jlong[count];
			for (unsigned int j = 0; j < count; ++j)
				temparrayout[j] = thegraph->nodes[i].outNodeIds[j];
			env->SetLongArrayRegion(arrayoutnode, 0, count, temparrayout);
			delete[] temparrayout;
			env->SetObjectField(tempobj, jfoutnodeids, arrayoutnode);
			env->SetObjectArrayElement(nodeArray, i, tempobj);
			env->DeleteLocalRef(jStr);
		}
	}

	return (jobjectArray)nodeArray;
}

bool Java_ReadDataFromObject(JNIEnv *env, jobject obj, struct settings *set){
	jclass TestClass = env->GetObjectClass(obj);
	jfieldID theboolean = env->GetFieldID(TestClass, "testneo4j", "Z");
	jboolean tneo4j = env->GetBooleanField(obj, theboolean);
	bool testneo4j = (bool)(tneo4j == JNI_TRUE);
	
	jfieldID location = env->GetFieldID(TestClass, "neo4loc", "Ljava/lang/String;");
	jstring jloc = (jstring)env->GetObjectField(obj, location);
	set->neo4j = env->GetStringUTFChars(jloc, NULL);

	location = env->GetFieldID(TestClass, "neo4user", "Ljava/lang/String;");
	jstring juser = (jstring)env->GetObjectField(obj, location);
	set->neouser = env->GetStringUTFChars(juser, NULL);

	location = env->GetFieldID(TestClass, "neo4pass", "Ljava/lang/String;");
	jstring jpass = (jstring)env->GetObjectField(obj, location);
	set->neopass = env->GetStringUTFChars(jpass, NULL);

	location = env->GetFieldID(TestClass, "sqdbfile", "Ljava/lang/String;");
	jstring jdbf = (jstring)env->GetObjectField(obj, location);
	set->dbfil = env->GetStringUTFChars(jdbf, NULL);
	if (set->dbfil.length() == 0)
		set->dbfil = "data.db";

	return testneo4j;
}

