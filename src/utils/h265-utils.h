/*
 mediastreamer2 library - modular sound and video processing and streaming
 Copyright (C) 2015  Belledonne Communications <info@belledonne-communications.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#pragma once

#include <cstdint>
#include <ortp/str_utils.h>

namespace mediastreamer {

class H265NaluType {
public:
	H265NaluType() = default;
	H265NaluType(uint8_t value);

	operator uint8_t() const {return _value;}

	bool isVcl() const {return _value < 32;}

	static const H265NaluType Ap;
	static const H265NaluType Fu;

private:
	uint8_t _value = 0;
};

class H265NaluHeader {
public:
	H265NaluHeader() = default;
	H265NaluHeader(const uint8_t *header) {parse(header);}

	void setType(H265NaluType type) {_type = type;}
	H265NaluType getType() const {return _type;}

	void setLayerId(uint8_t layerId);
	uint8_t getLayerId() const {return _layerId;}

	void setTid(uint8_t tid);
	uint8_t getTid() const {return _tid;}

	void parse(const uint8_t *header);
	mblk_t *forge() const;

private:
	H265NaluType _type;
	uint8_t _layerId = 0;
	uint8_t _tid = 0;
};

class H265FuHeader {
public:
	enum class Position {
		Start,
		Middle,
		End
	};

	H265FuHeader() = default;
	H265FuHeader(const uint8_t *header) {parse(header);}

	void setPosition(Position pos) {_pos = pos;}
	Position getPosition() const {return _pos;}

	void setType(H265NaluType type) {_type = type;}
	H265NaluType getType() const {return _type;}

	void parse(const uint8_t *header);
	mblk_t *forge() const;

private:
	Position _pos = Position::Start;
	H265NaluType _type;
};

}
