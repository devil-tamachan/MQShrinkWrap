

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "MQPlugin.h"
#include "MQWidget.h"

BOOL ShrinkWrap(MQDocument doc);


#include <iostream>
#include <list>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;

typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef K::Direction_3 Direction;
typedef K::Vector_3 Vector3;

typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;
typedef boost::optional<Tree::Intersection_and_primitive_id<Ray>::Type> Ray_intersection;


//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	//プラグインとしては特に必要な処理はないので、何もせずにTRUEを返す
    return TRUE;
}

//---------------------------------------------------------------------------
//  MQGetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT void MQGetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	*Product = 0xA8BEE201;
	*ID      = 0xCC9DA490;
}

//---------------------------------------------------------------------------
//  MQGetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQGetPlugInName(void)
{
	// プラグイン名
	return "MQShrinkWrap           Copyright(C) 2017, tamachan";
}

//---------------------------------------------------------------------------
//  MQGetPlugInType
//    プラグインのタイプを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT int MQGetPlugInType(void)
{
	// 選択部変形用プラグインである
	return MQPLUGIN_TYPE_SELECT;
}

//---------------------------------------------------------------------------
//  MQEnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumString(int index)
{
	switch(index){
	case 0: return "ShrinkWrap";
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQModifySelect
//    メニューから選択されたときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQModifySelect(int index, MQDocument doc)
{
	switch(index){
	case 0: return ShrinkWrap(doc);
	}
	return FALSE;
}


#include <atlconv.h>

class ShrinkDialog : public MQDialog
{
public:
	ShrinkDialog(MQWindowBase& parent);
	~ShrinkDialog();

  void MakeObjList(MQDocument doc)
  {
    USES_CONVERSION;//For A2W
    m_objIdx.clear();
    int numObj = doc->GetObjectCount();
    for(int i=0;i<numObj;i++)
    {
      MQObject o = doc->GetObject(i);
      if(o==NULL)continue;
      m_objIdx.push_back(i);
      std::string name = o->GetName();
      std::wstring wname(A2W(name.c_str()));
      combo_guideobj->AddItem(wname);
    }
  }

  int GetSelectObjIdx()
  {
    int i = combo_guideobj->GetCurrentIndex();
    if(i>=0 && i<m_objIdx.size())return m_objIdx[i];
    return -1;
  }

	MQComboBox *combo_guideobj;
  std::vector<int> m_objIdx;
	MQComboBox *combo_mode;
};

ShrinkDialog::ShrinkDialog(MQWindowBase& parent) : MQDialog(parent)
{
	SetTitle(L"ShrinkWrap");

	MQFrame *mainFrame = CreateHorizontalFrame(this);

	MQFrame *paramFrame = CreateHorizontalFrame(mainFrame);
	paramFrame->SetMatrixColumn(2);
	
	CreateLabel(paramFrame, L"吸着先");
	combo_guideobj = CreateComboBox(paramFrame);
	
	CreateLabel(paramFrame, L"モード");
	combo_mode = CreateComboBox(paramFrame);
  combo_mode->AddItem(L"最短面上に吸着");
  combo_mode->AddItem(L"X軸のみ可変");
  combo_mode->AddItem(L"Y軸のみ可変");
  combo_mode->AddItem(L"Z軸のみ可変");
  combo_mode->AddItem(L"カメラ方向のみ可変");
  combo_mode->SetCurrentIndex(0);

	MQFrame *sideFrame = CreateVerticalFrame(mainFrame);

	MQButton *okbtn = CreateButton(sideFrame, L"OK");
	okbtn->SetDefault(true);
	okbtn->SetModalResult(MQDialog::DIALOG_OK);

	MQButton *cancelbtn = CreateButton(sideFrame, L"Cancel");
	cancelbtn->SetCancel(true);
	cancelbtn->SetModalResult(MQDialog::DIALOG_CANCEL);
}

ShrinkDialog::~ShrinkDialog()
{
}

void AABB_AddMQObj(std::list<Triangle> &triangles, MQDocument doc, MQObject o)
{
  if(o==NULL)return;
  int numV = o->GetVertexCount();
  int numF = o->GetFaceCount();
  for(int k=0;k<numF;k++)
  {
    int numFV = o->GetFacePointCount(k);
    if(numFV<3)continue;
    
    std::vector<int> index(numFV);
    o->GetFacePointArray(k, &(*index.begin()));
    
    int numTri = numFV - 2;
    std::vector<int> indices(numTri*3);
    if(numFV==3)
    {
      indices[0] = 0;
      indices[1] = 1;
      indices[2] = 2;
    } else {
      std::vector<MQPoint> pts(numFV);
      for(int i=0; i<numFV; i++)
      {
        pts[i] = o->GetVertex(index[i]);
      }
      doc->Triangulate(&(*pts.begin()), numFV, &(*indices.begin()), numTri*3);
    }
    for(int m=0;m<numTri;m++)
    {
      MQPoint p;
      p = o->GetVertex(index[indices[m*3+0]]);
      Point a(p.x, p.y, p.z);
      p = o->GetVertex(index[indices[m*3+1]]);
      Point b(p.x, p.y, p.z);
      p = o->GetVertex(index[indices[m*3+2]]);
      Point c(p.x, p.y, p.z);
      
      triangles.push_back(Triangle(a,b,c));
    }
  }
}

void MoveClosestPoint(Tree &tree, MQDocument doc, int ignoreObjIdx = -1)
{
  for(int i=0;i<doc->GetObjectCount();i++)
  {
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    if(o->GetVisible()==0)continue;
    if(o->GetLocking()==1)continue;
    if(i==ignoreObjIdx)continue;
    
    int numV = o->GetVertexCount();
    for(int j=0;j<numV;j++)
    {
      if(doc->IsSelectVertex(i, j)==FALSE)continue;
      
      MQPoint mp_query = o->GetVertex(j);
      Point p_query(mp_query.x, mp_query.y, mp_query.z);
      Point closest_point = tree.closest_point(p_query);
      mp_query.x = closest_point[0];
      mp_query.y = closest_point[1];
      mp_query.z = closest_point[2];
      o->SetVertex(j, mp_query);
    }
  }
}

void MoveXYZAxis(Tree &tree, MQDocument doc, int modexyz, int ignoreObjIdx = -1)
{
  Direction d1, d2;
  switch(modexyz)
  {
  case 0:
    d1 = Direction(Vector3(1.0, 0.0, 0.0));
    d2 = Direction(Vector3(-1.0, 0.0, 0.0));
    break;
  case 1:
    d1 = Direction(Vector3(0.0, 1.0, 0.0));
    d2 = Direction(Vector3(0.0, -1.0, 0.0));
    break;
  default:
    d1 = Direction(Vector3(0.0, 0.0, 1.0));
    d2 = Direction(Vector3(0.0, 0.0, -1.0));
    break;
  }
  for(int i=0;i<doc->GetObjectCount();i++)
  {
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    if(o->GetVisible()==0)continue;
    if(o->GetLocking()==1)continue;
    if(i==ignoreObjIdx)continue;
    
    int numV = o->GetVertexCount();
    for(int j=0;j<numV;j++)
    {
      if(doc->IsSelectVertex(i, j)==FALSE)continue;
      
      MQPoint mp_query = o->GetVertex(j);
      Point p_query(mp_query.x, mp_query.y, mp_query.z);
      Ray r1(p_query, d1);
      Ray r2(p_query, d2);
      Ray_intersection intersection1 = tree.first_intersection(r1);
      Ray_intersection intersection2 = tree.first_intersection(r2);
      Point *p1 = NULL, *p2 = NULL;
      double len1 = DBL_MAX, len2 = DBL_MAX;
      if(intersection1){
        if(boost::get<Point>(&(intersection1->first))){
          p1 =  boost::get<Point>(&(intersection1->first) );
          Vector3 v1(p_query, *p1);
          len1 = v1.squared_length();
        }
      }
      if(intersection2){
        if(boost::get<Point>(&(intersection2->first))){
          p2 =  boost::get<Point>(&(intersection2->first) );
          Vector3 v2(p_query, *p2);
          len2 = v2.squared_length();
        }
      }
      if(p1==NULL && p2==NULL)continue;

      Point *pz = (len1<len2)?p1:p2;
      mp_query.x = (*pz)[0];
      mp_query.y = (*pz)[1];
      mp_query.z = (*pz)[2];
      o->SetVertex(j, mp_query);
    }
  }
}

void MoveCameraAxis(Tree &tree, MQDocument doc, int ignoreObjIdx = -1)
{
  MQScene scene = doc->GetScene(0);
  MQPoint mqcp = scene->GetCameraPosition();
  Point p_camera(mqcp.x, mqcp.y, mqcp.z);

  for(int i=0;i<doc->GetObjectCount();i++)
  {
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    if(o->GetVisible()==0)continue;
    if(o->GetLocking()==1)continue;
    if(i==ignoreObjIdx)continue;
    
    int numV = o->GetVertexCount();
    for(int j=0;j<numV;j++)
    {
      if(doc->IsSelectVertex(i, j)==FALSE)continue;
      
      MQPoint mp_query = o->GetVertex(j);
      Point p_query(mp_query.x, mp_query.y, mp_query.z);
      Vector3 v1(p_camera, p_query);
      Vector3 v2(p_query, p_camera);
      
      Ray r1(p_query, v1.direction());
      Ray r2(p_query, v2.direction());
      Ray_intersection intersection1 = tree.first_intersection(r1);
      Ray_intersection intersection2 = tree.first_intersection(r2);
      Point *p1 = NULL, *p2 = NULL;
      double len1 = DBL_MAX, len2 = DBL_MAX;
      if(intersection1){
        if(boost::get<Point>(&(intersection1->first))){
          p1 =  boost::get<Point>(&(intersection1->first) );
          Vector3 v1(p_query, *p1);
          len1 = v1.squared_length();
        }
      }
      if(intersection2){
        if(boost::get<Point>(&(intersection2->first))){
          p2 =  boost::get<Point>(&(intersection2->first) );
          Vector3 v2(p_query, *p2);
          len2 = v2.squared_length();
        }
      }
      if(p1==NULL && p2==NULL)continue;

      Point *pz = (len1<len2)?p1:p2;
      mp_query.x = (*pz)[0];
      mp_query.y = (*pz)[1];
      mp_query.z = (*pz)[2];
      o->SetVertex(j, mp_query);
    }
  }
}

BOOL ShrinkWrap(MQDocument doc)
{
  int guideObjIdx = 0;

	MQWindow mainwin = MQWindow::GetMainWindow();
	ShrinkDialog dlg(mainwin);
  dlg.MakeObjList(doc);
	if(dlg.Execute() != MQDialog::DIALOG_OK){
		return FALSE;
	}
  guideObjIdx = dlg.GetSelectObjIdx();
  if(guideObjIdx<0)return FALSE;

  int mode = dlg.combo_mode->GetCurrentIndex();
  
  MQObject o = doc->GetObject(guideObjIdx);
  if(o==NULL || o->GetVisible()==0)return FALSE;
  
  std::list<Triangle> triangles;
  AABB_AddMQObj(triangles, doc, o);
  Tree tree(triangles.begin(),triangles.end());
  switch(mode)
  {
  case 0:
    MoveClosestPoint(tree, doc, guideObjIdx);
    break;
  case 1:
  case 2:
  case 3:
    MoveXYZAxis(tree, doc, mode-1, guideObjIdx);
    break;
  case 4:
    MoveCameraAxis(tree, doc, guideObjIdx);
    break;
  }

	MQ_RefreshView(NULL);
	
	return TRUE;
}

