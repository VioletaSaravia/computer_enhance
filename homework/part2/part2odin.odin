package part2

import "core:time"
import "core:bytes"
import "core:fmt"
import la "core:math/linalg"
import "core:math/rand"
import "core:os"
import "core:strconv"
import "core:strings"

HaversinePair :: struct {
	x0, y0, x1, y1: f64,
}

Haversine :: proc(points: HaversinePair, radius: f64) -> f64 {
	dLat := la.to_radians(points.y1 - points.y0)
	dLon := la.to_radians(points.x1 - points.x0)
	lat1 := la.to_radians(points.y0)
	lat2 := la.to_radians(points.y1)

	a :=
		la.pow(la.sin(dLat / 2.0), 2) + la.cos(lat1) * la.cos(lat2) * la.pow(la.sin(dLon / 2.0), 2)
	c := 2.0 * la.asin(la.sqrt(a))
	return radius * c
}

SumHaversines :: proc(pairs: []HaversinePair) -> (sum: f64) {
	sumCoef := 1 / f64(len(pairs))
	for i in pairs {
		sum += sumCoef * Haversine(i, 6372.8)
	}
	return
}

HaversineParsingState :: enum {
	OpenBracket,
	OpenBrace,
	KeyStart,
	Key,
	Colon,
	Value,
	Comma,
	CloseBrace,
}

JsonKeyValue :: struct {
	key:   string,
	value: f64,
}

ParseHaversineJson :: proc(
	file: []byte,
	count: int = 0,
) -> (
	result: [dynamic]HaversinePair,
	success: bool,
) {
	if count != 0 {
		reserve(&result, count + 1)
	}
	state: HaversineParsingState = .OpenBracket
	curKey, _ := strings.builder_make()
	curVal, _ := strings.builder_make()

	curPair: HaversinePair
	curKv: JsonKeyValue

	for b in file {
		if bytes.contains({'\n', '\t', '\r', ' '}, {b}) && state != .Key {
			continue
		}

		switch state {
		case .OpenBracket:
			if b != '[' {
				return
			} else {
				state = .OpenBrace
			}
		case .OpenBrace:
			if b != '{' {
				return
			} else {
				state = .KeyStart
			}
		case .KeyStart:
			if b != '\"' {
				return
			} else {
				state = .Key
			}
		case .Key:
			if b == '\"' {
				state = .Colon
			} else {
				strings.write_byte(&curKey, b)
			}
		case .Colon:
			if b == ':' {
				curKv.key = strings.to_string(curKey)
				state = .Colon
				state = .Value
			} else {
				return
			}
		case .Value:
			switch b {
			case ',':
				state = .Comma
			case '}':
				state = .CloseBrace
			case:
				strings.write_byte(&curVal, b)
				continue
			}

			val := strings.to_string(curVal)
			curKv.value = strconv.atof(val)
			switch curKv.key {
			case "x0":
				curPair.x0 = curKv.value
			case "y0":
				curPair.y0 = curKv.value
			case "x1":
				curPair.x1 = curKv.value
			case "y1":
				curPair.y1 = curKv.value
			}
			strings.builder_reset(&curKey)
			strings.builder_reset(&curVal)
		case .Comma:
			state = b == '\"' ? .Key : .Value
		case .CloseBrace:
			append(&result, curPair)
			if b == ',' {
				state = .OpenBrace
			} else if b == ']' {
				success = true
				return
			}
		}
	}
	return
}

GenerateHaversineJson :: proc(count: int, path: string) {
	handle, _ := os.open(path, os.O_CREATE | os.O_WRONLY)
	buf: [32]byte
	defer os.close(handle)

	os.write_string(handle, "[\n")
	for i in 0 ..< count {
		os.write_string(handle, "\t{\n")

		os.write_string(handle, "\t\t\"x0\": ")
		os.write(handle, strconv.generic_ftoa(buf[:], rand.float64() * 160 - 80, 'f', 2, 64))
		os.write_string(handle, ",\n")

		os.write_string(handle, "\t\t\"y0\": ")
		os.write(handle, strconv.generic_ftoa(buf[:], rand.float64() * 160 - 80, 'f', 2, 64))
		os.write_string(handle, ",\n")

		os.write_string(handle, "\t\t\"x1\": ")
		os.write(handle, strconv.generic_ftoa(buf[:], rand.float64() * 160 - 80, 'f', 2, 64))
		os.write_string(handle, ",\n")

		os.write_string(handle, "\t\t\"y1\": ")
		os.write(handle, strconv.generic_ftoa(buf[:], rand.float64() * 160 - 80, 'f', 2, 64))
		os.write_string(handle, "\n")

		os.write_string(handle, "\t}")
		if i + 1 != count {
			os.write_string(handle, ",")
		}
		os.write_string(handle, "\n")
	}
	os.write_string(handle, "]\n")
}

main :: proc() {
	pairCount: int
	for i, n in os.args {
		if n == 1 {
			pairCount = strconv.atoi(i)
		}
	}

	GenerateHaversineJson(pairCount if pairCount != 0 else 100, "input.json")
	file, _ := os.read_entire_file("input.json")
	pairs, ok := ParseHaversineJson(file, pairCount)
	if !ok {return}
	sum := SumHaversines(pairs[:])
	fmt.println(sum)
}
