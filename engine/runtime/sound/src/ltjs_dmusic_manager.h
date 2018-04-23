#ifndef LTJS_USE_DIRECT_MUSIC8


#include <memory>
#include "iltdirectmusic.h"


namespace ltjs
{


class DMusicManager :
	public ILTDirectMusicMgr
{
public:
	interface_version(ILTDirectMusicMgr, 0);
	declare_interface(DMusicManager)


	DMusicManager();


	DMusicManager(
		const DMusicManager& that) = delete;

	DMusicManager& operator=(
		const DMusicManager& that) = delete;

	DMusicManager(
		DMusicManager&& that);

	~DMusicManager();


	LTRESULT Init() override;

	LTRESULT Term() override;

	LTRESULT InitLevel(
		const char* sWorkingDirectory,
		const char* sControlFileName,
		const char* sDefine1 = nullptr,
		const char* sDefine2 = nullptr,
		const char* sDefine3 = nullptr) override;

	LTRESULT TermLevel() override;

	LTRESULT Play() override;

	LTRESULT Stop(
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	LTRESULT Pause(
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	LTRESULT UnPause() override;

	LTRESULT SetVolume(
		const long nVolume) override;

	LTRESULT ChangeIntensity(
		const int nNewIntensity,
		const LTDMEnactTypes nStart = LTDMEnactInvalid) override;

	LTRESULT PlaySecondary(
		const char* sSecondarySegment,
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	LTRESULT StopSecondary(
		const char* sSecondarySegment = nullptr,
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	LTRESULT PlayMotif(
		const char* sStyleName,
		const char* sMotifName,
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	LTRESULT StopMotif(
		const char* sStyleName,
		const char* sMotifName = NULL,
		const LTDMEnactTypes nStart = LTDMEnactDefault) override;

	int GetCurIntensity() override;

	LTDMEnactTypes StringToEnactType(
		const char* sName) override;

	void EnactTypeToString(
		LTDMEnactTypes nType,
		char* sName) override;

	int GetNumIntensities() override;

	int GetInitialIntensity() override;

	int GetInitialVolume() override;

	int GetVolumeOffset() override;


private:
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr pimpl_;
}; // DMusicManager


} // ltjs


#endif // !LTJS_USE_DIRECT_MUSIC8
