

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "MQPlugin.h"
#include "MQWidget.h"

#include <shlwapi.h>

BOOL ShrinkWrap(MQDocument doc);

#include <boost/filesystem.hpp>


#include <iostream>
#include <list>

#include "RunCmd.h"

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



class ShrinkDialog : public MQDialog
{
public:
  ShrinkDialog(MQWindowBase& parent);
  ~ShrinkDialog();

  BOOL uiChanged(MQWidgetBase *sender, MQDocument doc)
  {
    UpdateEnable(doc);
    return FALSE;
  }
  
  void UpdateEnable(MQDocument doc)
  {
    MakeObjList(doc);
  }

  void MakeObjList(MQDocument doc)
  {
    bool bVisibleObjOnly = check_visibleObjOnly->GetChecked();
    
    combo_guideobj->ClearItems();
    m_objIdx.clear();
    int numObj = doc->GetObjectCount();
    for(int i=0;i<numObj;i++)
    {
      MQObject o = doc->GetObject(i);
      if(o==NULL)continue;
      if(bVisibleObjOnly && o->GetVisible()==0)continue;
      m_objIdx.push_back(i);
      combo_guideobj->AddItem(o->GetNameW());
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
  MQCheckBox *check_visibleObjOnly;
};

ShrinkDialog::ShrinkDialog(MQWindowBase& parent) : MQDialog(parent)
{
  SetTitle(L"ShrinkWrap");

  MQFrame *mainFrame = CreateHorizontalFrame(this);

  MQFrame *paramFrame = CreateHorizontalFrame(mainFrame);
  paramFrame->SetMatrixColumn(2);

  CreateLabel(paramFrame, L"吸着先");
  combo_guideobj = CreateComboBox(paramFrame);

  CreateLabel(paramFrame, L"\"吸着先オブジェクト\"から非表示オブジェを除く");
  check_visibleObjOnly = CreateCheckBox(paramFrame);
  check_visibleObjOnly->SetChecked(true);
  check_visibleObjOnly->AddChangedEvent(this, &ShrinkDialog::uiChanged);

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


bool _WriteAABB_ByMQObject(MQDocument doc, MQObject o, FILE *fp)
{
  if(fputc('A', fp)==EOF)return false;
  if(fputc('A', fp)==EOF)return false;
  if(fputc('B', fp)==EOF)return false;
  if(fputc('B', fp)==EOF)return false;
  if(fputc('T', fp)==EOF)return false;
  if(fputc(0x0, fp)==EOF)return false;
  if(fputc(0x0, fp)==EOF)return false;
  
  float x,y,z;
  
  if(o==NULL)return false;
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
      int m3 = m*3;
      for(int n=0;n<3;n++)
      {
        p = o->GetVertex(index[indices[m3+n]]);
        x = p.x;
        y = p.y;
        z = p.z;
        if(fwrite(&x, sizeof(float), 1, fp)!=1)return false;
        if(fwrite(&y, sizeof(float), 1, fp)!=1)return false;
        if(fwrite(&z, sizeof(float), 1, fp)!=1)return false;
      }
    }
  }
  return true;
}
bool WriteAABB_ByMQObject(MQDocument doc, MQObject o, const char *path)
{
  FILE *fp = fopen(path, "wb");
  if(fp==NULL)return false;
  bool bRet = _WriteAABB_ByMQObject(doc, o, fp);
  fclose(fp);
  return bRet;
}
bool WriteAABB_ByMQObject(MQDocument doc, MQObject o, const wchar_t *path)
{
  FILE *fp = _wfopen(path, L"wb");
  if(fp==NULL)return false;
  bool bRet = _WriteAABB_ByMQObject(doc, o, fp);
  fclose(fp);
  return bRet;
}

bool _WriteMoveVertex_SelectedVertex(MQDocument doc, FILE *fp, int ignoreObjIdx = -1)
{
  if(fputc('A', fp)==EOF)return false;
  if(fputc('A', fp)==EOF)return false;
  if(fputc('B', fp)==EOF)return false;
  if(fputc('B', fp)==EOF)return false;
  if(fputc('V', fp)==EOF)return false;
  if(fputc(0x0, fp)==EOF)return false;
  if(fputc(0x0, fp)==EOF)return false;
  
  float x, y, z;
  for(int oi=0;oi<doc->GetObjectCount();oi++)
  {
    MQObject o = doc->GetObject(oi);
    if(o==NULL)continue;
    if(o->GetVisible()==0)continue;
    if(o->GetLocking()==1)continue;
    if(oi==ignoreObjIdx)continue;
    
    int numV = o->GetVertexCount();
    for(int vi=0;vi<numV;vi++)
    {
      if(doc->IsSelectVertex(oi, vi)==FALSE)continue;
      
      MQPoint mp_query = o->GetVertex(vi);
      x = mp_query.x;
      y = mp_query.y;
      z = mp_query.z;
      
      if(fwrite(&oi, sizeof(int), 1, fp)!=1)return false;
      if(fwrite(&vi, sizeof(int), 1, fp)!=1)return false;
      if(fwrite(&x, sizeof(float), 1, fp)!=1)return false;
      if(fwrite(&y, sizeof(float), 1, fp)!=1)return false;
      if(fwrite(&z, sizeof(float), 1, fp)!=1)return false;
    }
  }
  return true;
}
bool WriteMoveVertex_SelectedVertex(MQDocument doc, const char *path, int ignoreObjIdx = -1)
{
  FILE *fp = fopen(path, "wb");
  if(fp==NULL)return false;
  bool bRet = _WriteMoveVertex_SelectedVertex(doc, fp, ignoreObjIdx);
  fclose(fp);
  return bRet;
}
bool WriteMoveVertex_SelectedVertex(MQDocument doc, const wchar_t *path, int ignoreObjIdx = -1)
{
  FILE *fp = _wfopen(path, L"wb");
  if(fp==NULL)return false;
  bool bRet = _WriteMoveVertex_SelectedVertex(doc, fp, ignoreObjIdx);
  fclose(fp);
  return bRet;
}


std::string GetShrinkWrapPathA()
{
  char path[_MAX_PATH+16];
  path[0] = NULL;
  bool bRet = GetDllDirA(path, _MAX_PATH);
  if(!bRet)return "ShrinkWrap.exe";
  std::string ret = path;
  return ret+"\\ShrinkWrap.exe";
}

std::wstring GetShrinkWrapPathW()
{
  wchar_t path[_MAX_PATH+16];
  path[0] = NULL;
  bool bRet = GetDllDirW(path, _MAX_PATH);
  if(!bRet)return L"ShrinkWrap.exe";
  std::wstring ret = path;
  return ret+L"\\ShrinkWrap.exe";
}


bool _ReadMoveVertex(MQDocument doc, FILE *fp)
{
  if(fgetc(fp)!='A')return false;
  if(fgetc(fp)!='A')return false;
  if(fgetc(fp)!='B')return false;
  if(fgetc(fp)!='B')return false;
  if(fgetc(fp)!='V')return false;
  if(fgetc(fp)!=0x0)return false;
  if(fgetc(fp)!=0x0)return false;
  float x,y,z;
  int oi, vi;
  
  while(1)
  {
    for(int pi=0;pi<3;pi++)
    {
      if(fread(&oi, sizeof(int), 1, fp)!=1)return true;
      if(fread(&vi, sizeof(int), 1, fp)!=1)return false;
      if(fread(&x, sizeof(float), 1, fp)!=1)return false;
      if(fread(&y, sizeof(float), 1, fp)!=1)return false;
      if(fread(&z, sizeof(float), 1, fp)!=1)return false;
      
      MQObject o = doc->GetObject(oi);
      if(o==NULL || o->GetLocking()==1)return false;
      int numVertex = o->GetVertexCount();
      if(vi<0 || vi>=numVertex)return false;
      o->SetVertex(vi, MQPoint(x,y,z));
    }
  }
  return true;
}
bool ReadMoveVertex(MQDocument doc, const char *path)
{
  FILE *fp = fopen(path, "rb");
  if(fp==NULL)return false;
  bool bRet = _ReadMoveVertex(doc, fp);
  fclose(fp);
  return bRet;
}
bool ReadMoveVertex(MQDocument doc, const wchar_t *path)
{
  FILE *fp = _wfopen(path, L"rb");
  if(fp==NULL)return false;
  bool bRet = _ReadMoveVertex(doc, fp);
  fclose(fp);
  return bRet;
}

BOOL ShrinkWrap(MQDocument doc)
{
  int guideObjIdx = 0;

  MQWindow mainwin = MQWindow::GetMainWindow();
  ShrinkDialog dlg(mainwin);
  dlg.UpdateEnable(doc);
  if(dlg.Execute() != MQDialog::DIALOG_OK){
    return FALSE;
  }
  guideObjIdx = dlg.GetSelectObjIdx();
  if(guideObjIdx<0)return FALSE;

  int mode = dlg.combo_mode->GetCurrentIndex();
  
  MQObject o = doc->GetObject(guideObjIdx);
  if(o==NULL /*|| o->GetVisible()==0*/)return FALSE;
  
  bool bRet = false;
  
  std::wstring ppath = GetShrinkWrapPathW();
  std::wstring inpath = MyGetTempFilePathW();
  std::wstring aabbpath = MyGetTempFilePathW();
  std::wstring outpath = MyGetTempFilePathW();
  
  bRet = WriteAABB_ByMQObject(doc, o, aabbpath.c_str());
  if(!bRet)
  {
    _wremove(aabbpath.c_str());
    return FALSE;
  }
  
  bRet = WriteMoveVertex_SelectedVertex(doc, inpath.c_str(), guideObjIdx);
  if(!bRet)
  {
    _wremove(aabbpath.c_str());
    _wremove(inpath.c_str());
    return FALSE;
  }

  MQScene scene = doc->GetScene(0);
  MQPoint mqcp = scene->GetCameraPosition();
  std::wstring strCameraPos = L"";
  long double xll = mqcp.x;
  long double yll = mqcp.y;
  long double zll = mqcp.z;
  if(mode==4)strCameraPos = L" -c "+std::to_wstring(xll)+L" "+std::to_wstring(yll)+L" "+std::to_wstring(zll)+L" ";
  
  long long modell = mode;
  std::wstring cmd = L"\""+ppath+L"\" --in \""+inpath+L"\" --target \""+aabbpath+L"\" --mode "+std::to_wstring(modell)+strCameraPos+L" --out \""+outpath+L"\"";

  DWORD result = RunCmdW(cmd);
  
  if(result!=0)
  {
    OutputDebugStringA("ShrinkWrap.exe failed!");
    _wremove(outpath.c_str());
    _wremove(inpath.c_str());
    _wremove(aabbpath.c_str());
    return FALSE;
  }
  
  bool ret = ReadMoveVertex(doc, outpath.c_str());
  
  _wremove(outpath.c_str());
  _wremove(inpath.c_str());
  _wremove(aabbpath.c_str());
  
  if(!ret)
  {
    OutputDebugStringA("outfile corrupted!!");
    return FALSE;
  }

  MQ_RefreshView(NULL);

  return TRUE;
}

