#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "RenderHandler.h"
#include "MovementComponent.h"
#include "PhysicsComponent.h"

class GameObject
{
private:
	// ID
	UINT m_id;
	bool m_enabled = true;
	std::string m_modelName;
	int m_shaderType;

	// Components
	std::unique_ptr< MovementComponent > m_movementComponent;
	std::unique_ptr< PhysicsComponent > m_physicsComponent;
	XMFLOAT3 m_rotationOffset;
	// Other Components
	int m_otherComponentIndex;

	bool m_audioComponent;
	char* m_audioFileName;
	bool m_loopingAudio;
	float m_volumeAudio;

	// Renderer
	RenderObjectKey m_renderKey;
	RenderHandler* m_renderHandler;
public:
	GameObject();
	~GameObject();

	// Initialization
	void initialize(std::string modelName, UINT id, ShaderStates shaderState = ShaderStates::PHONG, std::vector<MeshData>* meshData = nullptr);

	// Getters
	std::string getModelName() const;
	std::string getModelNameAndId() const;

	ShaderStates getShaderType() const;

	XMVECTOR getScale() const;
	XMFLOAT3 getScaleF3() const;

	XMVECTOR getRotation() const;
	XMFLOAT3 getRotationF3() const;

	XMVECTOR getPosition() const;
	XMFLOAT3 getPositionF3() const;

	RenderObjectKey getKey() const;

	// Setters
	// - Material
	void setTextures(TexturePaths textures);
	void setTextures(TexturePathsPBR textures);

	void setMaterial(PS_MATERIAL_BUFFER material);
	void setMaterial(PS_MATERIAL_PBR_BUFFER material);
	void setEnabled(bool enabled);

	void fillMeshData(std::vector<MeshData>* meshes);

	// - Movement
	void setScale(XMVECTOR newScale);
	void setScale(XMFLOAT3 newScale);

	void setRotation(XMVECTOR rotation);
	void setRotation(XMFLOAT3 rotation);
	void rotate(XMFLOAT3 rotation);

	void setPosition(XMVECTOR newPosition);
	void setPosition(XMFLOAT3 newPosition);

	// Update
	void update(double dt);
};

#endif // !GAMEOBJECT_H