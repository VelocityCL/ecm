#include "global.hpp"
#include "logger/logger.hpp"
#include "api.hpp"

void api::get_places()
{
	api::places = {};
	api::details = {};
	api::places_done = false;
	api::place = {};
	api::all_countries = {};
	api::station = {};
	api::station = {};

	std::thread([]
	{
		httplib::Client cli(API_URL);

		if (httplib::Result res = cli.Get(PLACES_ENDPOINT))
		{
			std::cout << "[ INFO ] [ " << __FUNCNAME__ << " ] " << "Accessing " << API_URL << PLACES_ENDPOINT << std::endl;

			if (res->status == 200)
			{
				api::places = nl::json::parse(res->body);

				//Check version matching for future updates
				std::int32_t version = api::places["apiVersion"].get<std::int32_t>();
				if (version != 1)
				{
					SDL_ShowSimpleMessageBox(0, "Radio.Garten Places", &logger::va("apiVersion was expected to be 1 (got %i)", version)[0], global::window);
					global::shutdown = true;
				}

				std::string version_hash = api::places["version"].dump();
				if (std::strcmp(VERSION_HASH, &version_hash[0]))
				{
					std::cout << "[ WARNING ] [ " << __FUNCNAME__ << " ] " <<
					"Hash was expected to be " << VERSION_HASH << " (got " << &version_hash[0] << ")" << std::endl;
				}

				nl::json data = nl::json::parse(api::places["data"].dump());
				for (nl::detail::iteration_proxy_value<nl::detail::iter_impl<nl::json>>& i : data["list"].items())
				{
					//Get string
					std::string id = i.value()["id"].dump();
					std::string country = i.value()["country"].dump();
					std::string city = i.value()["title"].dump();

					//Rem ""
					id.erase(std::remove(id.begin(), id.end(), '\"'), id.end());
					country.erase(std::remove(country.begin(), country.end(), '\"'), country.end());
					city.erase(std::remove(city.begin(), city.end(), '\"'), city.end());

					//Check if we already have this country
					bool found = false;
					for (auto j : api::all_countries)
					{
						if (!std::strcmp(&j[0], &country[0]))
						{
							found = true;
							break;
						}
					}

					//If we don't add it
					if (!found)
					{
						api::all_countries.emplace_back(country);
					}

					//Add
					api::place.emplace_back(place_t{ country, city, id });
				}

				//Sort alphabetically
				std::sort(api::all_countries.begin(), api::all_countries.end());

				//Finish
				api::places_done = true;
				api::places = {};
			}
			else
			{
				std::cout << "[ ERROR ] [ " << __FUNCNAME__ << " ] " << "An error occured when gathering places data!" << std::endl;
			}
		}
	}).detach();
}

void api::get_details(const place_t& place_in)
{
	api::details = {};
	api::detail_done = false;
	api::station = {};

	std::thread([place_in]
	{
		httplib::Client cli(API_URL);

		if (httplib::Result res = cli.Get(PLACE_DETAIL_ENDPOINT(&place_in.id[0])))
		{
			std::cout << "[ INFO ] [ " << __FUNCNAME__ << " ] " << "Accessing " << API_URL << PLACE_DETAIL_ENDPOINT(&place_in.id[0]) << std::endl;

			if (res->status == 200)
			{
				api::details = nl::json::parse(res->body);

				//Check version matching for future updates
				std::int32_t version = api::details["apiVersion"].get<std::int32_t>();
				if (version != 1)
				{
					SDL_ShowSimpleMessageBox(0, "Radio.Garten Details", &logger::va("apiVersion was expected to be 1 (got %i)", version)[0], global::window);
					global::shutdown = true;
				}

				std::string version_hash = api::details["version"].dump();
				if (std::strcmp(VERSION_HASH, &version_hash[0]))
				{
					std::cout << "[ WARNING ] [ " << __FUNCNAME__ << " ] " <<
					"Hash was expected to be " << VERSION_HASH << " (got " << &version_hash[0] << ")" << std::endl;
				}

				nl::json data = nl::json::parse(api::details["data"].dump());

				for (auto& i : data["content"][0]["items"])
				{
					std::string id = i["href"].dump();
					std::string title = i["title"].dump();

					id.erase(std::remove(id.begin(), id.end(), '\"'), id.end());
					title.erase(std::remove(title.begin(), title.end(), '\"'), title.end());

					if (std::strcmp(&id[0], "null"))
					{
						api::station.emplace_back(station_t{ title, id.substr(id.size() - 8), place_in });
					}

				}

				//Finish
				api::detail_done = true;
				api::details = {};
			}
			else
			{
				std::cout << "[ ERROR ] [ " << __FUNCNAME__ << " ] " << "An error occured when gathering station data!" << std::endl;
			}
		}
	}).detach();
}

void api::get_station(const std::string& id)
{
	api::stations_done = false;

	std::thread([id]
		{
			httplib::Client cli(API_URL);

			if (httplib::Result res = cli.Get(STATION_ENDPOINT(&id[0])))
			{
				std::cout << "[ INFO ] [ " << __FUNCNAME__ << " ] " << "Accessing " << API_URL << STATION_ENDPOINT(&id[0]) << std::endl;
				if (res->status == 200)
				{
					api::stations = nl::json::parse(res->body);

					//Check version matching for future updates
					std::int32_t version = api::stations["apiVersion"].get<std::int32_t>();
					if (version != 1)
					{
						SDL_ShowSimpleMessageBox(0, "Radio.Garten Station", &logger::va("apiVersion was expected to be 1 (got %i)", version)[0], global::window);
						global::shutdown = true;
					}

					std::string version_hash = api::stations["version"].dump();
					if (std::strcmp(VERSION_HASH, &version_hash[0]))
					{
						std::cout << "[ WARNING ] [ " << __FUNCNAME__ << " ] " <<
						"Hash was expected to be " << VERSION_HASH << " (got " << &version_hash[0] << ")" << std::endl;
					}

					//Finish
					api::stations_done = true;
					api::stations = {};
				}
				else
				{
					std::cout << "[ ERROR ] [ " << __FUNCNAME__ << " ] " << "An error occured when gathering station data!" << std::endl;
				}
			}
		}).detach();
}

void api::filter_place(const std::string& key)
{
	api::filtered_place = {};

	for (auto place : api::place)
	{
		//Look for city, country, or id with this string
		if (place.city.find(key) != std::string::npos || place.country.find(key) != std::string::npos || place.id.find(key) != std::string::npos)
		{
			api::filtered_place.emplace_back(place);
		}
	}
}

std::string api::get_final_redirect(const std::string& entry_url)
{
	std::string domain = entry_url.substr(0, strlen("http://radio.garden"));
	std::string access = entry_url.substr(strlen("http://radio.garden"), entry_url.length() - strlen("http://radio.garden"));
	
	httplib::Client cli(&domain[0]);/*  */

	if(httplib::Result res = cli.Get(&access[0]))
	{
		std::cout << "[ INFO ] [ " << __FUNCNAME__ << " ] " << "Accessing " << domain << access << std::endl;

		cli.set_follow_location(true);
		if(res = cli.Get("/"))
		{
			std::cout << "Status" << res->status << std::endl;
		}
	}

	return std::string("");
}

nl::json api::places;
std::vector<place_t> api::place;
std::vector<place_t> api::filtered_place;
bool api::places_done = true;

nl::json api::details;
bool api::detail_done = true;

nl::json api::stations;
std::vector<station_t> api::station;
bool api::stations_done = true;

std::string api::current_place_id = "N/A";

std::vector<std::string> api::all_countries;