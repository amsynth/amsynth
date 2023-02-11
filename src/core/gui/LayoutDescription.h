/*
 *  LayoutDescription.h
 *
 *  Copyright (c) 2022 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

//
// Parses an amsynth layout.ini file
//
// - A [layout] section
//   - background=  name of the background image
//   - resources=   names of the resource sections (deprecated)
//
// - A [resource_name] section for each resource file
//   - file=        name of the image file
//   - width=       pixel width of each frame
//   - height=      pixel height of each frame
//   - frames=      number of frames in the image
//
// - A [parameter_name] section for each parameter in amsynth
//   - param_name=  repeats the paramter name (deprecated)
//   - type=        button, knob, or popup
//   - resource=    references a [resource_name] section
//   - pos_x=       pixel x position (relative to background)
//   - pos_y=       pixel y position (relative to background)
//
class LayoutDescription {
public:
	LayoutDescription(const std::string &file) {
		try {
			auto sections = parseIniFile(file);
			background = sections.at("layout").at("background");
			auto resources = parseResources(sections);
			controls = parseControls(sections, resources);
		} catch (const std::exception &ex) {
			std::cerr << ex.what() << std::endl;
		}
	}

	struct Resource {
		std::string file;
		int width, height, frames;
	};

	struct Control {
		std::string type;
		int x, y;
		Resource resource;
	};

	using ControlMap = std::unordered_map<std::string, Control>;

	std::string background;
	ControlMap controls;

private:
	using StringMap = std::unordered_map<std::string, std::string>;
	using SectionMap = std::unordered_map<std::string, StringMap>;
	using ResourceMap = std::unordered_map<std::string, Resource>;

	static SectionMap parseIniFile(std::string file) {
		SectionMap sections;
		std::ifstream stream(file);
		if (!stream) {
			return sections;
		}
		std::string name, line;
		while (std::getline(stream, line)) {
			if (line.empty() || line[0] == '#' || line[0] == ';') {
				continue;
			}
			if (line[0] == '[') {
				auto end = line.find_first_of(']');
				if (end == std::string::npos) {
					continue;
				}
				name = line.substr(1, end - 1);
			}
			auto end = line.find_first_of("=:");
			if (end != std::string::npos) {
				auto key = line.substr(0, end);
				auto value = line.substr(end + 1, line.size());
				sections[name][key] = value;
			}
		}
		return sections;
	}

	static ResourceMap
	parseResources(const SectionMap &sections) {
		ResourceMap resources;
		for (const auto &entry: sections) {
			auto name = entry.first;
			auto values = entry.second;
			if (values.find("file") != values.end()) {
				resources[name] = {
					.file = values.at("file"),
					.width = std::stoi(values.at("width")),
					.height = std::stoi(values.at("height")),
					.frames = std::stoi(values.at("frames")),
				};
			}
		}
		return resources;
	}

	static ControlMap
	parseControls(const SectionMap &sections, const ResourceMap &resources) {
		ControlMap controls;
		for (const auto &entry: sections) {
			auto name = entry.first;
			auto values = entry.second;
			if (values.find("resource") != values.end()) {
				controls[name] = {
					.type = values.at("type"),
					.x = std::stoi(values.at("pos_x")),
					.y = std::stoi(values.at("pos_y")),
					.resource = resources.at(values.at("resource"))
				};
			}
		}
		return controls;
	}
};
