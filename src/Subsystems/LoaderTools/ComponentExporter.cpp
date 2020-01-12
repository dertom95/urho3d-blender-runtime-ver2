#include "ComponentExporter.h"

#ifdef GAME_ENABLE_COMPONENT_EXPORTER

#include "base64.h"
#include <Urho3D/Container/Sort.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>

Urho3DNodeTreeExporter::Urho3DNodeTreeExporter(Context* context, ExportMode exportMode)
    : Object(context),
      m_exportMode(exportMode)
{
}

void Urho3DNodeTreeExporter::AddComponentHashToFilterList(const StringHash& componentHash)
{
    m_listOfComponents.Insert(componentHash);
}

void Urho3DNodeTreeExporter::AddSuperComponentHashToFilterList(const StringHash& superComponentHash)
{
    m_listOfSuperClasses.Insert(superComponentHash);
}

bool Urho3DNodeTreeExporter::CheckSuperTypes(const TypeInfo* type)
{
    for (auto superType : m_listOfSuperClasses){
        if (!type->IsTypeOf(superType)){
            return false;
        }
    }
    return true;
}


bool CompareString(const String& a,const String& b){
    return a < b;
}

void Urho3DNodeTreeExporter::ProcessFileSystem()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    FileSystem* fs = GetSubsystem<FileSystem>();

    materialFiles.Clear();
    techniqueFiles.Clear();
    textureFiles.Clear();
    modelFiles.Clear();

    for (String resDir : cache->GetResourceDirs()){
        Vector<String> dirFiles;
        for (String path : m_materialFolders){
            String dir = resDir+path;
            fs->ScanDir(dirFiles,dir,"*.xml",SCAN_FILES,true);
            for (String foundMaterial : dirFiles){
                auto materialResourceName = path+"/"+foundMaterial;
                Material* material = cache->GetResource<Material>(materialResourceName);
                if (material){
                    materialFiles.Push(materialResourceName);
                }
            }
        }


        // grab techniques from the specified technique folders
        for (String path : m_techniqueFolders){
            String dir = resDir+path;
            fs->ScanDir(dirFiles,dir,"*.xml",SCAN_FILES,true);
            for (String foundTechnique : dirFiles){
                auto techiqueResourceName = path+"/"+foundTechnique;
                Technique* technique = cache->GetResource<Technique>(techiqueResourceName);
                if (technique){
                    techniqueFiles.Push(techiqueResourceName);
                }
            }
        }


        // grab models from the specified model folder. all files with .mdl extension are considered a mesh
        for (String path : m_modelFolders){
            String dir = resDir+path;
            fs->ScanDir(dirFiles,dir,"*.mdl",SCAN_FILES,true);
            for (String foundModel : dirFiles){
                auto modelResourceName = path+"/"+foundModel;
                modelFiles.Push(modelResourceName);
            }
        }

        for (String path : m_animationFolders){
            String dir = resDir+path;
            fs->ScanDir(dirFiles,dir,"*.ani",SCAN_FILES,true);
            for (String foundAnim : dirFiles){
                auto animResourceName = path+"/"+foundAnim;
                animationFiles.Push(animResourceName);
            }
        }

        // grab textures from the specified texture folders
        for (String path : m_textureFolders){
            String dir = resDir+path;
            fs->ScanDir(dirFiles,dir,"*.jpg",SCAN_FILES,true);
            for (String foundTexture : dirFiles){
                ExportPath p;
                p.absFilepath = dir +"/"+foundTexture;
                p.resFilepath = path+"/"+foundTexture;
                textureFiles.Push(p);
            }
            fs->ScanDir(dirFiles,dir,"*.png",SCAN_FILES,true);
            for (String foundTexture : dirFiles){
                ExportPath p;
                p.absFilepath = dir +"/"+foundTexture;
                p.resFilepath = path+"/"+foundTexture;
                textureFiles.Push(p);
            }
            fs->ScanDir(dirFiles,dir,"*.dds",SCAN_FILES,true);
            for (String foundTexture : dirFiles){
                ExportPath p;
                p.absFilepath = dir +"/"+foundTexture;
                p.resFilepath = path+"/"+foundTexture;
                textureFiles.Push(p);
            }
        }

    }

    Sort(materialFiles.Begin(),materialFiles.End(),CompareString);
    Sort(techniqueFiles.Begin(),techniqueFiles.End(),CompareString);
   // Sort(textureFiles.Begin(),techniqueFiles.End(),CompareString);
    Sort(modelFiles.Begin(),modelFiles.End(),CompareString);
    Sort(animationFiles.Begin(),animationFiles.End(),CompareString);
}

JSONObject Urho3DNodeTreeExporter::ExportMaterials()
{
    const String treeID="urho3dmaterials";


    JSONObject tree;

    tree["id"]=treeID;
    tree["name"]="Tree "+treeID;
    tree["icon"]="COLOR_RED";

    JSONArray nodes;

    {
        // --------------------------------
        // --- PREDEFINED MATERIAL NODE ---
        // --------------------------------
        JSONObject predefMaterialNode;
        NodeSetData(predefMaterialNode,treeID+"__predefMaterialNode","PredefMaterial","Material");

        JSONArray enumElems;
        for (String matName : materialFiles){
            StringHash hash(matName);
            String id(hash.Value() % 10000000);
            NodeAddEnumElement(enumElems,matName,matName,"Material "+matName,"MATERIAL",id);
            URHO3D_LOGINFOF("MATERIAL: %s",matName.CString());
        }
        NodeAddPropEnum(predefMaterialNode,"Material",enumElems,false,"0");

        nodes.Push(predefMaterialNode);
    }

    {
        // --------------------------------
        // ---      MATERIAL NODE       ---
        // --------------------------------
        JSONObject materialNode;

        NodeSetData(materialNode,treeID+"__materialNode","Material","Material");
        NodeAddOutputSocket(materialNode,"defs",SOCK_BOOL);

        JSONArray cullModeEnum;
        NodeAddEnumElement(cullModeEnum,"ccw");
        NodeAddEnumElement(cullModeEnum,"cw");
        NodeAddEnumElement(cullModeEnum,"none");
        // cull-mode
        NodeAddPropEnum(materialNode,"cull",cullModeEnum,false,"0");
        // shadowcull-mode
        NodeAddPropEnum(materialNode,"shadowcull",cullModeEnum,false,"0");

        JSONArray fillEnum;
        NodeAddEnumElement(fillEnum,"solid");
        NodeAddEnumElement(fillEnum,"wireframe");
        NodeAddEnumElement(fillEnum,"point");
        // fill-mode
        NodeAddPropEnum(materialNode,"fill",fillEnum,false,"0");

        nodes.Push(materialNode);
    }

    {
        // --------------------------------
        // ---  DepthBias NODE   ---
        // --------------------------------
        JSONObject depthBiasNode;

        NodeSetData(depthBiasNode,treeID+"__DepthBiasNode","DepthBias","Material");
        NodeAddInputSocket(depthBiasNode,"Material",SOCK_BOOL);
        NodeAddProp(depthBiasNode,"constant",NT_FLOAT,"0");
        NodeAddProp(depthBiasNode,"slopescaled",NT_FLOAT,"0");

        nodes.Push(depthBiasNode);
    }

    {
        // --------------------------------
        // ---      PARAMETER NODE      ---
        // --------------------------------
        JSONObject parameterNode;

        NodeSetData(parameterNode,treeID+"__parameterNode","Parameter","Material");
        NodeAddInputSocket(parameterNode,"Material",SOCK_BOOL);

        JSONArray predefNames;
        NodeAddEnumElement(predefNames,"MatDiffColor");
        NodeAddEnumElement(predefNames,"MatSpecColor");
        NodeAddEnumElement(predefNames,"MatEmissiveColor");
        NodeAddEnumElement(predefNames,"MatEnvMapColor");
        NodeAddEnumElement(predefNames,"UOffset");
        NodeAddEnumElement(predefNames,"VOffset");
        NodeAddEnumElement(predefNames,"Roughness");
        NodeAddEnumElement(predefNames,"Metallic");

        NodeAddPropEnum(parameterNode,"name",predefNames,true,"0");

        NodeAddProp(parameterNode,"value",NT_STRING,"0");

        nodes.Push(parameterNode);
    }

    {
        // --------------------------------
        // ---  CUSTOM PARAMETER NODE   ---
        // --------------------------------
        JSONObject customParameterNode;

        NodeSetData(customParameterNode,treeID+"__customParameterNode","CustomParameter","Material");
        NodeAddInputSocket(customParameterNode,"Material",SOCK_BOOL);
        NodeAddProp(customParameterNode,"key",NT_STRING,"");
        NodeAddProp(customParameterNode,"value",NT_STRING,"");

        nodes.Push(customParameterNode);
    }

    {
        // --------------------------------
        // ---     Standard PARAMS       ---
        // --------------------------------
        JSONObject customParameterNode;

        NodeSetData(customParameterNode,treeID+"__standardParams","StandardParams","Material");
        NodeAddInputSocket(customParameterNode,"Material",SOCK_BOOL);
        NodeAddProp(customParameterNode,"MatDiffColor",NT_COLOR,"(1,1,1,1)");
        NodeAddProp(customParameterNode,"MatSpecColor",NT_COLOR,"(0.0,0.0,0.0,1)");
        NodeAddProp(customParameterNode,"MatEmissiveColor",NT_COLOR,"(0,0,0,0)");
        NodeAddProp(customParameterNode,"UOffset",NT_FLOAT,"1",ST_FACTOR);
        NodeAddProp(customParameterNode,"VOffset",NT_FLOAT,"1",ST_FACTOR);

        nodes.Push(customParameterNode);
    }

    {
        // --------------------------------
        // ---        PBS PARAMS        ---
        // --------------------------------
        JSONObject customParameterNode;

        NodeSetData(customParameterNode,treeID+"__pbsParams","PBSParams","Material");
        NodeAddInputSocket(customParameterNode,"Material",SOCK_BOOL);
        NodeAddProp(customParameterNode,"MatDiffColor",NT_COLOR,"(1,1,1,1)");
        NodeAddProp(customParameterNode,"MatSpecColor",NT_COLOR,"(0.0,0.0,0.0,1)");
        NodeAddProp(customParameterNode,"MatEmissiveColor",NT_COLOR,"(0,0,0,0)");
        NodeAddProp(customParameterNode,"MatEnvMapColor",NT_COLOR,"(1,1,1,1)");
        NodeAddProp(customParameterNode,"Roughness",NT_FLOAT,"0.5",ST_FACTOR);
        NodeAddProp(customParameterNode,"Metallic",NT_FLOAT,"0",ST_FACTOR);
        NodeAddProp(customParameterNode,"UOffset",NT_FLOAT,"1",ST_FACTOR);
        NodeAddProp(customParameterNode,"VOffset",NT_FLOAT,"1",ST_FACTOR);

        nodes.Push(customParameterNode);
    }

    {
        JSONObject advancedMaterialNode;
        NodeSetData(advancedMaterialNode, treeID+"__advancedMaterial","MaterialAdvanced","Material" );
        NodeAddInputSocket(advancedMaterialNode,"Material",SOCK_BOOL);
        NodeAddProp(advancedMaterialNode,"vsdefines",NT_STRING,"");
        NodeAddProp(advancedMaterialNode,"psdefines",NT_STRING,"");
        NodeAddProp(advancedMaterialNode,"renderOrder",NT_INT,"128");
        NodeAddProp(advancedMaterialNode,"occlusion",NT_BOOL,"true");
        NodeAddProp(advancedMaterialNode,"alphaToCoverage",NT_BOOL,"false");
        NodeAddProp(advancedMaterialNode,"lineAntialias",NT_BOOL,"false");

        nodes.Push(advancedMaterialNode);
    }

    {
        // --------------------------------
        // ---      Technique NODE       ---
        // --------------------------------
        JSONObject techniqueNode;
        NodeSetData(techniqueNode,treeID+"__techniqueNode","technique","Material");

        // dropdown to choose techniques available from the resource-path
        JSONArray enumElems;
        for (String techniqueName : techniqueFiles){
            StringHash hash(techniqueName);
            String id(hash.Value() % 10000000);
            String category = "Misc";

            if (techniqueName.Contains("Vegetation",false)){
                category = "Misc";
            }
            else if (techniqueName.Contains("PBR",false)){
                category = "PBR";
            }
            else if (techniqueName.Contains("NoTexture",false)){
                category = "NoTexture";
            } else if (techniqueName.Contains("Diff",false)){
                    category = "Diff";
            }
            NodeAddEnumElement(enumElems,techniqueName,techniqueName,"Technique "+techniqueName,"COLOR",id,category);
        }

        NodeAddPropEnum(techniqueNode,"Technique",enumElems,true,"0");
        // quality
        NodeAddProp(techniqueNode,"quality",NT_INT,"0",ST_NONE,3,0,2);
        // lod distance
        NodeAddProp(techniqueNode,"distance",NT_INT,"0",ST_NONE,3,0,200);

        NodeAddInputSocket(techniqueNode,"material",SOCK_BOOL);

        nodes.Push(techniqueNode);
    }

    {
        // --------------------------------
        // ---      Texture NODE       ---
        // --------------------------------
        JSONObject textureNode;
        NodeSetData(textureNode,treeID+"__textureNode","texture","Material");


        JSONArray unitElems;
        NodeAddEnumElement(unitElems,"diffuse");
        NodeAddEnumElement(unitElems,"normal");
        NodeAddEnumElement(unitElems,"specular");
        NodeAddEnumElement(unitElems,"emissive");
        NodeAddEnumElement(unitElems,"environment");
        NodeAddEnumElement(unitElems,"0");
        NodeAddEnumElement(unitElems,"1");
        NodeAddEnumElement(unitElems,"2");
        NodeAddEnumElement(unitElems,"3");
        NodeAddEnumElement(unitElems,"4");
        NodeAddEnumElement(unitElems,"5");
        NodeAddPropEnum(textureNode,"unit",unitElems,false);


        // dropdown to choose textures available from the resource-path
        JSONArray enumElems;
        int counter=0;
        for (ExportPath texture : textureFiles){
            StringHash hash(texture.resFilepath);
            String id(hash.Value() % 10000000);

            NodeAddEnumElement(enumElems,texture.resFilepath,texture.resFilepath,texture.absFilepath,"COLOR",id);
        }
        NodeAddPropEnum(textureNode,"Texture",enumElems,true,"0",true);



        NodeAddInputSocket(textureNode,"material",SOCK_BOOL);

        nodes.Push(textureNode);
    }

    tree["nodes"]=nodes;

    return tree;
}

void Urho3DNodeTreeExporter::NodeSetData(JSONObject &node, const String &id, const String &name, const String category)
{
    node["id"]=id;
    node["name"]=name;
    node["category"]=category;
}

void Urho3DNodeTreeExporter::NodeAddProp(JSONObject &node, const String &name, NodeType type, const String& defaultValue, NodeSubType subType, int precission,float min,float max)
{
    JSONObject prop;
    prop["name"]=name;

    if (type>=NT_VECTOR2 && type<=NT_COLOR){
        prop["default"]="("+defaultValue.Replaced(" ",",")+")";
    } else {
        prop["default"]=defaultValue;
    }

    switch (type){
        case NT_BOOL: prop["type"] = "bool"; break;
        case NT_INT: prop["type"] = "int"; if (min>max) {prop["min"]=(int)min; prop["max"]=(int)max;}break;
        case NT_FLOAT: prop["type"] = "float"; if (min>max) {prop["min"]=min; prop["max"]=max;}prop["precision"]=precission;break;
        case NT_VECTOR2: prop["type"] = "vector2"; break;
        case NT_VECTOR3: prop["type"] = "vector3"; break;
        case NT_VECTOR4: prop["type"] = "vector4"; break;
        case NT_COLOR: prop["type"] = "color"; break;
        case NT_STRING: prop["type"] = "string"; break;
        default:  URHO3D_LOGERRORF("AddProp(%s): Unknown TYPE with int-value:%i",name.CString(),(int)type);
    }
    switch (subType){
        case ST_NONE: prop["subtype"] = "NONE"; break;
        case ST_PIXEL: prop["subtype"] = "PIXEL"; break;
        case ST_ANGLE: prop["subtype"] = "ANGLE"; break;
        case ST_DISTANCE: prop["subtype"] = "DISTANCE"; break;
        case ST_FACTOR: prop["subtype"] = "FACTOR"; break;
        case ST_TIME: prop["subtype"] = "TIME"; break;
        case ST_UNSIGNED: prop["subtype"] = "UNSIGNED"; break;
        default:  URHO3D_LOGERRORF("AddProp(%s): Unknown SUBTYPE with int-value:%i",name.CString(),(int)type);
    }



    if (!node.Contains("props")){
        JSONArray propsArray;
        node["props"]=propsArray;
    }

    // TODO: make this easier
    JSONArray props = node["props"].GetArray();
    props.Push(prop);
    node["props"]=props;
}

void Urho3DNodeTreeExporter::NodeAddPropEnum(JSONObject &node, const String &name, JSONArray &elements, bool categorized,const String &defaultValue, bool isPreview)
{
    JSONObject prop;
    prop["name"]=name;
    prop["type"]=isPreview?"enumPreview":"enum";
    prop["elements"]=elements;
    prop["default"]=defaultValue;
    prop["use_category"]=categorized?"True":"False";

    if (!node.Contains("props")){
        JSONArray propsArray;
        node["props"]=propsArray;
    }

    // TODO: make this easier
    JSONArray props = node["props"].GetArray();
    props.Push(prop);
    node["props"]=props;
}


void Urho3DNodeTreeExporter::NodeAddEnumElement(JSONArray &elements, const String &id, const String &name, const String &descr, const String &icon, const String& number, const String& category)
{
    JSONObject elem;
    elem["id"]=id;
    elem["name"]=name==""?id:name;
    elem["description"]=descr==""?id:descr;
    elem["icon"]=icon;
    elem["number"]=number;
    if (category!="")
        elem["category"]=category;
    elements.Push(elem);
}

void Urho3DNodeTreeExporter::AddMaterialFolder(const String &folder)
{
    m_materialFolders.Push(folder);
}

void Urho3DNodeTreeExporter::AddTechniqueFolder(const String& folder)
{
    m_techniqueFolders.Push(folder);
}

void Urho3DNodeTreeExporter::AddTextureFolder(const String& folder)
{
    m_textureFolders.Push(folder);
}

void Urho3DNodeTreeExporter::AddModelFolder(const String& folder)
{
    m_modelFolders.Push(folder);
}

void Urho3DNodeTreeExporter::AddAnimationFolder(const String& folder)
{
    m_animationFolders.Push(folder);
}

void Urho3DNodeTreeExporter::NodeAddSocket(JSONObject &node, const String &name, NodeSocketType type,bool isInputSocket)
{
    String socketlistName = isInputSocket ? "inputsockets" : "outputsockets";

    JSONObject socket;
    socket["name"]=name;
    switch(type){
        case SOCK_BOOL : socket["type"]="bool"; break;
        case SOCK_FLOAT : socket["type"]="float"; break;
        case SOCK_STRING : socket["type"]="string"; break;
        case SOCK_VECTOR : socket["type"]="vector"; break;
    }

    if (!node.Contains(socketlistName)){
        JSONArray sockets;
        node[socketlistName]=sockets;
    }

    // TODO: make this easier
    JSONArray sockets = node[socketlistName].GetArray();
    sockets.Push(socket);
    node[socketlistName]=sockets;
}

void Urho3DNodeTreeExporter::NodeAddInputSocket(JSONObject &node, const String &name, NodeSocketType type)
{
    NodeAddSocket(node,name,type,true);
}

void Urho3DNodeTreeExporter::NodeAddOutputSocket(JSONObject &node, const String &name, NodeSocketType type)
{
    NodeAddSocket(node,name,type,false);
}

String  Urho3DNodeTreeExporter::GetTypeCategory(const StringHash& hash,const String& defaultValue){
    const HashMap<String, Vector<StringHash> >& objectCategories = context_->GetObjectCategories();
    for (HashMap<String, Vector<StringHash> >::ConstIterator i = objectCategories.Begin(); i != objectCategories.End(); ++i)
    {
        if (i->second_.Contains(hash))
            return i->first_;
    }
    return defaultValue;
}

JSONObject Urho3DNodeTreeExporter::ExportComponents()
{
    const HashMap<StringHash, SharedPtr<ObjectFactory> >& objFactories = context_->GetObjectFactories();

    auto values = objFactories.Values();

    JSONObject tree;

    String treeID = "urho3dcomponents";

    tree["id"]=treeID;
    tree["name"]="Tree "+treeID;
    tree["icon"]="OUTLINER_OB_GROUP_INSTANCE";

    JSONArray nodes;

    Vector<Pair<String, unsigned> > sortedTypes;
    for (unsigned i = 0; i < values.Size(); ++i)
    {
        SharedPtr<ObjectFactory> val = values.At(i);

        // apply black- /whitelist-Filter
        if (    (InBlacklistMode() && (m_listOfComponents.Contains(val->GetType()) || CheckSuperTypes(val->GetTypeInfo())) )
             || (InWhiteListMode() && (!m_listOfComponents.Contains(val->GetType()) && !CheckSuperTypes(val->GetTypeInfo()) )) )
            continue;

        if (val.NotNull())
        {
            sortedTypes.Push(MakePair(val->GetTypeName(), i));
        }
    }
    Sort(sortedTypes.Begin(), sortedTypes.End());

    const HashMap<StringHash, SharedPtr<ObjectFactory> >& all = context_->GetObjectFactories();

    for (int i=0;i < sortedTypes.Size(); i++){

        auto objectFactoryName = sortedTypes[i].first_;
        JSONObject node;

        SharedPtr<ObjectFactory> val = *all[StringHash(objectFactoryName)];

        if (val->GetTypeInfo()->IsTypeOf(Component::GetTypeInfoStatic())){
       //     URHO3D_LOGINFOF("TYPE:%s\n",val->GetTypeName().CString());
            node["category"]=GetTypeCategory(val->GetTypeInfo()->GetType(),"Misc");
        } else {
       //     URHO3D_LOGINFOF("NO COMPONENT: TYPE:%s\n",val->GetTypeName().CString());
        }

        node["id"]=treeID+"__"+val->GetTypeName().Replaced(" ","_");
        node["name"]=val->GetTypeName();

        JSONArray props;

        auto attrs = context_->GetAttributes(val->GetTypeInfo()->GetType());
        if (attrs){
            for (int j = 0;j<attrs->Size();j++){
                auto attr = attrs->At(j);
              //  URHO3D_LOGINFOF("\tattr:%s\n", attr.name_.CString());


                if (attr.mode_ & AM_NOEDIT)
                    continue; // ignore no-edit attributes

                if (attr.name_.Contains('.'))
                    continue;

                JSONObject prop;

                // work around to use new prop-helpers
                bool alreadyAdded = false;

                prop["name"] = attr.name_;
                switch (attr.type_){
                    case VAR_BOOL :
                        NodeAddProp(node, attr.name_,NT_BOOL,attr.defaultValue_.ToString()); break;
                    case VAR_INT : {
                        if (!attr.enumNames_) {
                            NodeAddProp(node, attr.name_,NT_INT,attr.defaultValue_.ToString());
                        } else {
                            JSONArray elements;
                            for (int idx = 0; attr.enumNames_[idx] != NULL; idx++)
                            {
                                NodeAddEnumElement(elements,attr.enumNames_[idx],attr.enumNames_[idx]);
                            }
                            NodeAddPropEnum(node, attr.name_, elements,false,attr.defaultValue_.ToString());
                        }
                        break;
                    }

                    case VAR_FLOAT :
                        NodeAddProp(node, attr.name_,NT_FLOAT,attr.defaultValue_.ToString());break;
                    case VAR_STRING :
                        NodeAddProp(node, attr.name_,NT_STRING,attr.defaultValue_.ToString());break;
                    case VAR_COLOR :
                        NodeAddProp(node, attr.name_,NT_COLOR,attr.defaultValue_.ToString());break;
                    case VAR_VECTOR2 :
                        NodeAddProp(node, attr.name_,NT_VECTOR2,attr.defaultValue_.ToString());break;
                    case VAR_VECTOR3 :
                        NodeAddProp(node, attr.name_,NT_VECTOR3,attr.defaultValue_.ToString());break;
                    case VAR_VECTOR4 :
                        NodeAddProp(node, attr.name_,NT_VECTOR4,attr.defaultValue_.ToString());break;

                    case VAR_RESOURCEREF :
                        prop["type"]="string";
                        if (!attr.defaultValue_.GetResourceRef().type_){
                            prop["default"]="REF_UNKNOWN";
                        } else {
                            auto typeName = context_->GetTypeName(attr.defaultValue_.GetResourceRef().type_);
                            if (typeName=="Model"){
                                // dropdown to choose techniques available from the resource-path
                                JSONArray enumElems;
                                NodeAddEnumElement(enumElems,"None","None","No Mesh","MESH");
                                NodeAddEnumElement(enumElems,"__Node-Mesh","Node-Mesh","The node's current mesh","MESH","1");
                                NodeAddEnumElement(enumElems,"__Node-Col-Mesh","Node-Mesh-Col-Prefix","current meshname with col-prefixed","COLMESH","2");

                                for (String model : modelFiles){
                                    StringHash hash(model);
                                    String id(hash.Value() % 10000000);

                                    NodeAddEnumElement(enumElems,"Model;"+model,model,"Model "+model,"MESH",id);
                                }

                                NodeAddPropEnum(node,attr.name_,enumElems,true,"0");
                                alreadyAdded = true;
                            }
                            else if (typeName == "Animation")
                            {
                                // dropdown to choose techniques available from the resource-path
                                JSONArray enumElems;
                                NodeAddEnumElement(enumElems,"none","None","No Animation","ANIM");

                                for (String anim : animationFiles){
                                    StringHash hash(anim);
                                    String id(hash.Value() % 10000000);

                                    NodeAddEnumElement(enumElems,"Animation;"+anim,anim,"Animation "+anim,"ANIM",id);
                                }

                                NodeAddPropEnum(node,attr.name_,enumElems,true,"0");
                                alreadyAdded = true;

                            }
                            else if (typeName == "Texture2D")
                            {
                                // dropdown to choose textures available from the resource-path
                                JSONArray enumElems;
                                NodeAddEnumElement(enumElems,"none","None","No Texture","TEXTURE");

                                for (ExportPath tex : textureFiles){
                                    StringHash hash(tex.resFilepath);
                                    String id(hash.Value() % 10000000);

                                    NodeAddEnumElement(enumElems,"Texture;"+tex.resFilepath,tex.resFilepath,tex.absFilepath,"TEXTURE",id);
                                }

                                NodeAddPropEnum(node,attr.name_,enumElems,true,"0");
                                alreadyAdded = true;

                            }
                            else if (typeName == "Material")
                            {
                                // dropdown to choose techniques available from the resource-path
                                JSONArray enumElems;
                                NodeAddEnumElement(enumElems,"none","None","No Material","MATERIAL");

                                for (String mat : materialFiles){
                                    StringHash hash(mat);
                                    String id(hash.Value() % 10000000);

                                    NodeAddEnumElement(enumElems,"Material;"+mat,mat,"Material "+mat,"MATERIAL",id);
                                }

                                NodeAddPropEnum(node,attr.name_,enumElems,true,"0");
                                alreadyAdded = true;

                            }
                        }
                    break;
                    default:
                        URHO3D_LOGINFOF("[%s] Skipping attribute:%s",val->GetTypeName().CString(),attr.name_.CString());
                        continue;

                }

            }
        }

        nodes.Push(node);

    }
    tree["nodes"]=nodes;
    return tree;
}

JSONObject Urho3DNodeTreeExporter::ExportGlobalData(){
    JSONObject globalData;

    // dropdown to choose techniques available from the resource-path
    JSONArray techniques;
    for (String techniqueName : techniqueFiles){
        StringHash hash(techniqueName);
        String id(hash.Value() % 10000000);

        String category = "";
        if (techniqueName.Contains("NoTexture",false)){
            category = "NoTexture";
        } else if (techniqueName.Contains("Diff",false)){
                category = "Diff";
        }


        NodeAddEnumElement(techniques,techniqueName,techniqueName,"Technique "+techniqueName,"COLOR",id,category);
    }
    globalData["techniques"] = techniques;

    JSONArray textures;
    for (ExportPath texture : textureFiles){
        StringHash hash(texture.resFilepath);
        String id(hash.Value() % 10000000);

        NodeAddEnumElement(textures,texture.resFilepath,texture.resFilepath,texture.absFilepath,"COLOR",id);
    }
    globalData["textures"] = textures;

    return globalData;
}

void Urho3DNodeTreeExporter::Export(String filename,bool exportComponentTree,bool exportMaterialTree)
{

    ProcessFileSystem();

    auto globalData = ExportGlobalData();
    trees.Clear();
    if (exportComponentTree)
    {
        auto componentTree = ExportComponents();
        trees.Push(componentTree);
    }
    if (exportMaterialTree) {
        auto materialTree = ExportMaterials();
        trees.Push(materialTree);
    }

    if (!m_customUIFilenames.Empty()){
        FileSystem* fs = GetSubsystem<FileSystem>();
        JSONArray jsonCustomUIs;
        for (auto m_customUIFilename : m_customUIFilenames)
        {
            if (!fs->FileExists(m_customUIFilename)){
                URHO3D_LOGERRORF("File %s not found",m_customUIFilename.CString());
            } else {
                File file(context_);
                file.Open(m_customUIFilename);
                String allText="";
                while (!file.IsEof()){
                    String line = file.ReadLine()+"\n";
                    allText += line;
                }
                URHO3D_LOGINFO("READ FILE:");
                URHO3D_LOGINFO(allText.CString());

                auto e = base64_encode(reinterpret_cast<const unsigned char*>(allText.CString()),allText.Length());
                String enc(e.c_str());
                String dec(base64_decode(enc.CString()).c_str());


                jsonCustomUIs.Push(enc);
            }
        }
        fileRoot["customUI"] = jsonCustomUIs;
    }

    fileRoot["trees"]=trees;
    fileRoot["globalData"]=globalData;

    JSONFile file(context_);
    file.GetRoot() = fileRoot;
    file.SaveFile(filename);

}

void Urho3DNodeTreeExporter::AddCustomUIFile(const String &filename)
{
     m_customUIFilenames.Push(filename);
}
#endif
