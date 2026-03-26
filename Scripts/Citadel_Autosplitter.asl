state("Citadel") {}

init {
    vars.p = IntPtr.Zero;
}

update {
    if (vars.p == IntPtr.Zero) {
        foreach (ProcessModule m in game.Modules) {
            var s = new SignatureScanner(game, m.BaseAddress, m.ModuleMemorySize);
            vars.p = s.Scan(new SigScanTarget("37 13 37 13 37 13 37 13"));
            if (vars.p != IntPtr.Zero) {
                vars.w = new MemoryWatcherList {
                    new MemoryWatcher<double>(vars.p + 8) { Name = "t" },
                    new MemoryWatcher<bool>(vars.p + 16) { Name = "l" },
                    new MemoryWatcher<int>(vars.p + 17) { Name = "i" }
                };
                break;
            }
        }
        if (vars.p == IntPtr.Zero) return false;
    }
    vars.w.UpdateAll(game);
}

start { return vars.w["i"].Current == 0 && vars.w["t"].Current > 0 && vars.w["t"].Old == 0; }
split { return vars.w["i"].Current > vars.w["i"].Old; }
isLoading { return vars.w["l"].Current; }
gameTime { return TimeSpan.FromSeconds(vars.w["t"].Current); }
reset { return vars.w["i"].Current == 0 && vars.w["i"].Old > 0; }
