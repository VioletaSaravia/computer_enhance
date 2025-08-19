
struct Measurement
{
    cstr label;
    u64 measurement;
    u64 iterations;
};

#ifndef M_MAX
#define M_MAX 32
#endif

/*
    NOTE(Viole): Issues:
        - It can only take one measurement inside a loop (probably fine)
        - Profiling loops requires a call to EndLoop() (inevitable?)
        - Subprofiles need to be created manually and passed around, or made static (annoying)
        - .start and .name are unnecesary, they could just be the beginning measurement.
        - Subprofilers beyond the first layer don't work
*/
struct Profiler
{
    cstr name;
    u64 subLayer;
    u64 start;
    u64 count;
    bool ended;

    Measurement buf[M_MAX];

    FixedArray<u64, 16> childrenSlot; // bleh
    FixedArray<Profiler *, 16> children;

    Array<Profiler> childrenNEW;
    Array<Measurement> measurementsNEW;

    static Profiler New(cstr name = "")
    {
        return Profiler{
            .name = name,
            .start = ReadOSTimer(),
        };
    }

    void Add(cstr label = "")
    {
        if (count + 1 >= M_MAX)
            return;

        buf[count++] = {
            .label = label,
            .measurement = ReadOSTimer(),
            .iterations = buf[count].iterations + 1,
        };
    }

    void AddLoop(cstr label = "")
    {
        if (count + 1 >= M_MAX)
            return;

        buf[count] = {
            .label = label,
            .measurement = ReadOSTimer(),
            .iterations = buf[count].iterations + 1,
        };
    }

    void EndLoop()
    {
        count++;
    }

    void Add(Profiler *sub)
    {
        childrenSlot.Push(count);
        sub->subLayer = subLayer + 1;
        Add(sub->name);
        children.Push(sub);
        // sub.End();
    }

    void EndAndPrint()
    {
        if (ended)
        {
            return;
        }
        if (count == 0)
        {
            ended = true;
            return;
        }

        ended = true;

        f64 total = f64(buf[count - 1].measurement - start) / f64(GetOSTimerFreq());

        if (subLayer == 0)
        {
            printf("Finished profile %s in %.6f seconds:\n", name, total);
        }

        u64 child = 0;
        for (int i = 0; i < count; i++)
        {
            for (size_t j = 0; j < subLayer; j++)
            {
                printf("\t");
                if (j + 1 == subLayer)
                {
                    printf("  |->");
                }
            }

            f64 elapsedTotal = (f64)(buf[i].measurement - (i == 0 ? start : buf[i - 1].measurement));
            f64 elapsed = (elapsedTotal / (f64)buf[i].iterations) / (f64)(GetOSTimerFreq());

            if (buf[i].iterations < 2)
            {
                printf("  %02d) %-30s %12.6f seconds %8.2f%%\n", i + 1, buf[i].label, elapsed, (elapsed / total) * 100);
            }
            else
            {
                f64 elapsedTotalSecs = elapsedTotal / (f64)(GetOSTimerFreq());
                printf("  %02d) %-20s %.6f secs/loop (%4.2f secs) %8.2f%%\n", i + 1, buf[i].label, elapsed, elapsedTotalSecs, (elapsedTotalSecs / total) * 100);
            }

            if (child < children.len && childrenSlot.data[child] == i)
            {
                children.data[child++]->EndAndPrint();
            }
        }
    }

    ~Profiler()
    {
        if (subLayer == 0)
            EndAndPrint();
    }
};