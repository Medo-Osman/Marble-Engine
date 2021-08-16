#ifndef SHADERS_H
#define SHADERS_H

struct ShaderFiles
{
	LPCWSTR vs = L""; // Vertex Shader
	LPCWSTR hs = L""; // Hull Shader
	LPCWSTR ds = L""; // Domain Shader
	LPCWSTR gs = L""; // Geometry Shader
	LPCWSTR ps = L""; // Pixel Shader
	LPCWSTR cs = L""; // Compute Shader
};

enum ShaderStates {PHONG, PBR, NUM};
static const char* ShaderStatesNames[]{"Phong", "PBR"};

class Shaders
{
protected:
	// Device
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

private:
	// Shaders
	ComPtr< ID3D11VertexShader > m_vertexShader;
	ComPtr< ID3D11HullShader > m_hullShader;
	ComPtr< ID3D11DomainShader > m_domainShader;
	ComPtr< ID3D11GeometryShader > m_geometryShader;
	ComPtr< ID3D11PixelShader > m_pixelShader;
	ComPtr< ID3D11ComputeShader > m_computeShader;

	// Layout
	ComPtr< ID3D11InputLayout > m_layout;

	// Topology
	D3D_PRIMITIVE_TOPOLOGY m_topology;

public:
	Shaders();
	~Shaders();

	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ShaderFiles names, LayoutType layoutType = LayoutType::POS_NOR_TEX, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, bool streamOutput = false);

	void setShaders();
	void unbindShaders();
};

#endif // !SHADERS_H