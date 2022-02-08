#pragma once

namespace RenderingAPI
{
	enum class ApplicationType : int
	{
		DirectX12,
		Num_Of_Applications
	};
}
class Application
{
public:
	Application();
	Application(RenderingAPI::ApplicationType type);
	RenderingAPI::ApplicationType applicationType;
	virtual int Run();
};


