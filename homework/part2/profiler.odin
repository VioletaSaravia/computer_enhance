package part2

import "core:fmt"
import "core:time"

Measurement :: struct {
	label, file:           string,
	line:                  int,
	iterations:            u64,
	from, timeEx, timeInc: u64,
	bytesProcessed:        u64,
}

Profiler :: struct {
	name:         string,
	ended:        bool,
	start:        u64,
	printFile:    bool,
	measurements: [64]Measurement,
	queue:        [64]u64,
	queue_len:    int,
}

BlockFlag :: struct {
	parent: ^Profiler,
}

// global profiler instance
_global_profiler: Profiler
_IHateCpp: bool = false

ReadOSTimer :: proc() -> u64 {
	return auto_cast time.now()._nsec // placeholder
}

GetOSTimerFreq :: proc() -> u64 {
	return 1_000_000 // placeholder
}

Profiler_New :: proc(name: string, printFile: bool) {
	_global_profiler = Profiler {
		name      = name,
		ended     = false,
		start     = ReadOSTimer(),
		printFile = printFile,
		queue_len = 0,
	}
	_IHateCpp = true
}

Profiler_Get :: proc() -> ^Profiler {
	return &_global_profiler
}

Profiler_BeginBlock :: proc(id: u64, label: string, file: string, line: int, bytesProcessed: u64) {
	p := Profiler_Get()
	if id >= 64 {
		return
	}

	m := &p.measurements[id]
	time_now := ReadOSTimer()

	if p.queue_len > 0 {
		prev := &p.measurements[p.queue[p.queue_len - 1]]
		prev.timeEx += time_now - prev.from
		prev.timeInc += time_now - prev.from
	}

	m.from = time_now
	m.label = label
	m.file = file
	m.line = line
	m.bytesProcessed += bytesProcessed
	m.iterations += 1

	if p.queue_len < 64 {
		p.queue[p.queue_len] = id
		p.queue_len += 1
	}
}

Profiler_AddBytes :: proc(bytes: u64) {
	p := Profiler_Get()
	if p.queue_len == 0 {
		return
	}
	p.measurements[p.queue[p.queue_len - 1]].bytesProcessed += bytes
}

Profiler_BeginScopeBlock :: proc(
	id: u64,
	label: string,
	file: string,
	line: int,
	bytesProcessed: u64,
) -> BlockFlag {
	Profiler_BeginBlock(id, label, file, line, bytesProcessed)
	return BlockFlag{parent = Profiler_Get()}
}

Profiler_EndBlock :: proc() {
	p := Profiler_Get()
	if p.queue_len == 0 {
		return
	}

	now := ReadOSTimer()
	p.queue_len -= 1
	idx := p.queue[p.queue_len]
	m := &p.measurements[idx]

	m.timeEx += now - m.from
	m.timeInc += now - m.from

	if p.queue_len > 0 {
		prev := &p.measurements[p.queue[p.queue_len - 1]]
		prev.from = now
		prev.timeInc += now - m.from
	}
}

Profiler_End :: proc() {
	p := Profiler_Get()
	if !p.ended {
		p.ended = true
		_IHateCpp = false
	} else {
		return
	}

	totalTime := f64(ReadOSTimer() - p.start) / f64(GetOSTimerFreq())
	fmt.println("[INFO] Finished profiler ", p.name, " in ", totalTime, " seconds")
	fmt.println(
		" %-24s \t| %-25s \t| %-25s \t| %-12s",
		"Name[n]",
		"Time (Ex)",
		"Time (Inc)",
		"Bandwidth",
	)
	fmt.println(
		"---------------------------------------------------------------------------------------------------------------",
	)

	for i in 1 ..< 64 {
		next := p.measurements[i]
		if next.iterations == 0 {
			continue
		}

		nextTimeEx := f64(next.timeEx) / f64(GetOSTimerFreq())
		nextTimeInc := f64(next.timeInc) / f64(GetOSTimerFreq())

		if next.bytesProcessed == 0 {
			fmt.println(
				" %-20s [%d] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t|",
				next.label,
				next.iterations,
				nextTimeEx,
				(nextTimeEx / totalTime) * 100,
				nextTimeInc,
				(nextTimeInc / totalTime) * 100,
			)
		} else {
			fmt.println(
				" %-20s [%d] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t| %.3f MB/s",
				next.label,
				next.iterations,
				nextTimeEx,
				(nextTimeEx / totalTime) * 100,
				nextTimeInc,
				(nextTimeInc / totalTime) * 100,
				f64(next.bytesProcessed) / nextTimeEx / 1024.0 / 1024.0,
			)
		}
	}
}

// --- Macros equivalent in Odin ---
// PROFILER_NEW :: proc(name: string) {
// 	Profiler_New(name, false)
// }

// PROFILER_END :: proc() {
// 	Profiler_End()
// }

// PROFILE_BLOCK_BEGIN :: proc(name: string) {
// 	Profiler_BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__, 0)
// }

// PROFILE_ADD_BANDWIDTH :: proc(bytes: int) {
// 	Profiler_AddBytes(bytes)
// }

// PROFILE_BLOCK_END :: proc() {
// 	Profiler_EndBlock()
// }

// PROFILE_SCOPE :: proc(name: string) {
// 	_profilerFlag := Profiler_BeginScopeBlock(__COUNTER__ + 1, name, __FILE__, __LINE__, 0)
// }

// PROFILE_FUNCTION :: proc() {
// 	PROFILE_SCOPE(__PROC__)
// }

// PROFILE :: proc(name: string, code: any) {
// 	Profiler_BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__, 0)
// 	code
// 	Profiler_EndBlock()
// }
