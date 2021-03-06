//Mike Walters
//James Wojewoda
#include "LibDecl.h"
#include <io.h>
#include <algorithm>

cModel::cModel()
: Mtrls(NULL), Mesh(NULL), Textures(NULL),
  adjBuffer(NULL), mtrlBuffer(NULL),
  d3dxMaterials(NULL), numMtrls(0),
  translate(0.0f, 0.0f, 0.0f), rotate (0.0f, 0.0f, 0.0f),
  scale(1.0f, 1.0f,1.0f)//,matWorld(0,0,0)
{}
/////////////////////////////////////////////////////////////////////
cModel::~cModel()
{
    

    if (Textures != NULL)
    {
        for( DWORD i = 0; i < numMtrls; i++)
        {
            if (Textures[i] != NULL)
                Textures[i]->Release();
        }
        delete[] Textures;
    }
    
    if (Mesh != NULL)
        Mesh->Release();
}
//////////////////////////////////////////////////////////////
bool cModel::LoadModel(string filename, LPDIRECT3DDEVICE9 m_dev,string name){
	HRESULT result;
	

	result=D3DXLoadMeshFromX
		(
		filename.c_str(),
		D3DXMESH_SYSTEMMEM,
		m_dev,
		NULL,
		&mtrlBuffer,
		NULL,
		&numMtrls,
		&Mesh);

	if(result != D3D_OK)
	{
		fout<<"Failed Load Model"<<endl;
		return NULL;
	}



	LPD3DXMATERIAL d3dxMaterials= (LPD3DXMATERIAL)mtrlBuffer->GetBufferPointer();
	
	//Mtrls=new D3DMATERIAL9[numMtrls];
	Textures=new LPDIRECT3DTEXTURE9[numMtrls];

	for(DWORD i=0; i<numMtrls;i++){
		//d3dxMaterials[i].MatD3D.Ambient = d3dxMaterials[i].MatD3D.Diffuse;
		Mtrls.push_back (d3dxMaterials[i].MatD3D);
		
		

		Textures[i] = NULL;
		if(d3dxMaterials[i].pTextureFilename != NULL)
		{
			
			//string filename=d3dxMaterials[i].pTextureFilename;
			//if( FindFile(&filename))
			//{
				//fout<<"Find texture File" << endl;
				result=D3DXCreateTextureFromFile(
					m_dev,name.c_str(),&Textures[i]);
				if(result != D3D_OK)
				{
				fout<<"Failed at D3DXCREATETEXTURE"<<endl;
					return false;
				}
			//}
		}
	}
	mtrlBuffer->Release();

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////

void cModel::DrawModel(LPDIRECT3DDEVICE9 m_dev)
{
	
	D3DXMATRIX s,rx,ry,rz,t;
	D3DXMatrixScaling(&s,scale.x,scale.y,scale.z);
	D3DXMatrixTranslation(&t,translate.x,translate.y,translate.z);
	D3DXMatrixRotationX(&rx,rotate.x);
	D3DXMatrixRotationY(&ry,rotate.y);
	D3DXMatrixRotationZ(&rz,rotate.z);
	matWorld=s * rx * ry * rz * t;
	m_dev->SetTransform(D3DTS_WORLD,&matWorld);




	if(numMtrls==0){
		Mesh->DrawSubset(0);
	}
	else{
		for(DWORD i=0;i<numMtrls;i++)
		{
			m_dev->SetMaterial(&Mtrls[i]);

			if(Textures[i])
			{
				
				if(Textures[i]->GetType()== D3DRTYPE_TEXTURE)
				{
					
					D3DSURFACE_DESC desc;
					Textures[i]->GetLevelDesc(0,&desc);
					if(desc.Width > 0){
						m_dev->SetTexture(0,Textures[i]);
					}
				}
			}
			Mesh->DrawSubset(i);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
void cModel::SplitPath(const string& inputPath, string* pathOnly, string* filenameOnly)
{
    string fullPath( inputPath );
    replace( fullPath.begin(), fullPath.end(), '\\', '/');
    string::size_type lastSlashPos = fullPath.find_last_of('/');

    // check for there being no path element in the input
    if (lastSlashPos == string::npos)
    {
        *pathOnly="";
        *filenameOnly = fullPath;
    }
    else {
        if (pathOnly) {
            *pathOnly = fullPath.substr(0, lastSlashPos);
        }
        if (filenameOnly)
        {
            *filenameOnly = fullPath.substr( 
                lastSlashPos + 1,
                fullPath.size() - lastSlashPos - 1 );
        }
    }
}

bool cModel::DoesFileExist(const string &filename)
{
    return (_access(filename.c_str(), 0) != -1);
}

bool cModel::FindFile(string *filename)
{
    if (!filename) return false;
	fout << *filename << endl;
    //look for file using original filename and path
    if (DoesFileExist(*filename)) return true;
	
    //since the file was not found, try removing the path
    string pathOnly;
    string filenameOnly;
    SplitPath(*filename,&pathOnly,&filenameOnly);

    //is file found in current folder, without the path?
    if (DoesFileExist(filenameOnly))
    {
		fout<<"FindFile Loop"<<endl;
        *filename=filenameOnly;
        return true;
    }

    //not found
    return false;
}


BoundingBox::BoundingBox(cModel* m){
	model = m;
	
	
	min.x = 94.43f;
	min.y = 78.0f;
	min.z = 7.91f ;

	max.x = 0.0f;
	max.y = 0.0f;
	max.z = 0.0f;
	
}
bool BoundingBox::BoundingBoxSetUp(){

	
	ID3DXMesh* mesh;
	HRESULT hr=0;
    BYTE* v=0;
	mesh=model->getMesh();
	mesh->LockVertexBuffer(0,(void**)&v);

	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		mesh->GetNumVertices(),
		D3DXGetFVFVertexSize(mesh->GetFVF()),
		&min,
		&max);

	mesh->UnlockVertexBuffer();

	if( FAILED(hr) )
		return false;

	return true;
}
bool BoundingBox::isPointInside(D3DXVECTOR3 &p)
{
	
	if(p.x >= min.x && p.y >= min.y && p.z >= min.z &&
		p.x <= max.x && p.y <= max.y && p.z <= max.z)
	{ 
		return true;
	}
	else
	{
		return false;
	}
}
