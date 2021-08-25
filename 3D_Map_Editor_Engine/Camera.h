#ifndef CAMERA_H
#define CAMERA_H

#include "pch.h"

class Camera
{
private:
	// View and Projection
	XMMATRIX* m_projectionMatrix;
	XMMATRIX* m_viewMatrix;

	// Other
	bool m_isInitialized;
	float m_fov;

	// Constant Buffer
	struct PS_CAMERA_BUFFER
	{
		XMVECTOR cameraPosition;
		XMMATRIX viewInverseMatrix;
		XMMATRIX projInverseMatrix;
	};
	PS_CAMERA_BUFFER m_cameraData;
	Buffer<PS_CAMERA_BUFFER> m_cameraCBuffer;

	void updateConstantBuffer()
	{
		PS_CAMERA_BUFFER* cameraData = new PS_CAMERA_BUFFER(m_cameraData);
		m_cameraCBuffer.update(&cameraData);
	}

public:
	Camera()
	{
		m_projectionMatrix = nullptr;
		m_viewMatrix = nullptr;
		m_isInitialized = false;
		m_fov = 0.f;

		m_cameraData.cameraPosition = XMVectorSet(0.f, 0.f, -1.f, 1.f);
		m_cameraData.projInverseMatrix = XMMatrixIdentity();
		m_cameraData.viewInverseMatrix = XMMatrixIdentity();
	}
	~Camera()
	{
		delete m_projectionMatrix;
		delete m_viewMatrix;
	}

	// Initialization
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float fovAngle, float aspectRatio, float nearZ, float farZ)
	{
		m_fov = (fovAngle / 360.f) * XM_2PI;
		m_projectionMatrix = new XMMATRIX(XMMatrixPerspectiveFovLH(m_fov, aspectRatio, nearZ, farZ));
		m_cameraData.projInverseMatrix = XMMatrixInverse(nullptr, *m_projectionMatrix);

		m_viewMatrix = new XMMATRIX(XMMatrixIdentity());
		m_cameraCBuffer.initialize(device, deviceContext, &m_cameraData, BufferType::CONSTANT);
		updateViewMatrix(XMVectorSet(0.f, 0.f, -1.f, 1.f), XMVectorSet(0.f, 0.f, 0.f, 0.f));

		m_isInitialized = true;
	}

	// Getters
	XMMATRIX* getProjectionMatrixPtr() const { return m_projectionMatrix; }
	XMMATRIX* getViewMatrixPtr() const { return m_viewMatrix; }
	XMMATRIX getProjectionMatrix() const { return *m_projectionMatrix; }
	XMMATRIX getViewMatrix() const { return *m_viewMatrix; }
	XMFLOAT3 getCameraPositionF3() const { XMFLOAT3 temp; XMStoreFloat3(&temp, m_cameraData.cameraPosition); return temp; }
	bool isInitialized() const { return m_isInitialized; }
	float getFov() const { return m_fov; }

	// Update
	void updateViewMatrix(XMVECTOR position, XMVECTOR rotation)
	{
		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotation);
		// Look at
		XMVECTOR lookAt = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotationMatrix);
		// Add position to lookAt
		lookAt += position;
		// Up
		XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

		// Update View Matrix with new Rotation
		*m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);
		m_cameraData.viewInverseMatrix = XMMatrixInverse(nullptr, *m_viewMatrix);
		m_cameraData.cameraPosition = position;
		updateConstantBuffer();
	}

	ID3D11Buffer* const* getConstantBuffer()
	{
		return m_cameraCBuffer.GetAddressOf();
	}
};

#endif // !CAMERA_H