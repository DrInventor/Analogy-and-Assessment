#include "ie_nuim_cs_dri_interactROS_ROSInteract.h"

#include <string>
#include "ConsoleApplication2.h"

jobjectArray FillArray(JNIEnv *env, GraphProperties *thegraph);
bool Java_ReadDataFromObject(JNIEnv *env, jobject obj, struct settings *set);