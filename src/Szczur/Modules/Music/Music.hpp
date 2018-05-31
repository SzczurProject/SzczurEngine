#pragma once

#include <memory>

#include <boost/container/flat_map.hpp>
#include "Szczur/Utility/Convert/Hash.hpp"

#include "Szczur/Utility/Modules/Module.hpp"

#include "Szczur/Modules/Script/Script.hpp"

#include "MusicAssets.hpp"
#include "Playlist.hpp"

namespace rat 
{
	class Music : public Module<Script, AudioEffects>
	{ 
		using PlaylistContainer_t = boost::container::flat_map<Hash32_t, std::unique_ptr<Playlist>>;

	public:

		using PlayingMode = Playlist::PlayingMode;

	private:

		PlaylistContainer_t _playlists;
		MusicAssets _assets;
		
		Hash32_t _currentPlaylistKey = 0;

	public:

		Music(const std::string& assetsPath);
		~Music();

		void initScript();

		MusicAssets* getAssetsManager() { return &_assets; };

		void update(float deltaTime);

		void loadPlaylistFromJson(const std::string& fileName);

		void addPlaylist(const std::string& key, const std::vector<std::string>& newPlaylist);
		const Playlist::Container_t& getPlaylist(const std::string& key) { return _playlists[fnv1a_32(key.begin())]->getContainerRef(); };

		void addToPlaylist(const std::string& key, const std::string& fileName);
		void removeFromPlaylist(const std::string& key, const std::string& fileName = "");

		void play(const std::string& key, const std::string& fileName = "");
		void pause();
		void stop();

		RatMusic& getCurrentPlaying();
		RatMusic& get(const std::string& fileName);

		bool includes(const std::string& key, const std::string& fileName);

		void setPlayingMode(const std::string& key, PlayingMode mode);

		void setVolume(const std::string& key, float volume, const std::string& fileName = "");
		float getVolume(const std::string& fileName);

		template <typename T>
		T& getEffect(const std::string& fileName);

		template <typename T>
    	void cleanEffect(const std::string& fileName);
		void cleanEffects();

		void setGlobalEffects();

	private:

		void unLoad(const std::string& fileName);

	};

	template <typename T>
	T& Music::getEffect(const std::string& fileName)
	{
		return _assets.get(fileName).getEffect<T>();
	}

    template <typename T>
    void Music::cleanEffect(const std::string& fileName)
    {
        _assets.get(fileName).cleanEffect<T>();
    }

}


















