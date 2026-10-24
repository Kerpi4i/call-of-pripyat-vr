using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Win32;

internal static class Product {
    internal const string Version = "0.1.0-alpha.1";
    internal static string Root { get { return AppDomain.CurrentDomain.BaseDirectory.TrimEnd('\\'); } }
    internal static string Q(string value) { return "\"" + value.Replace("\"", "") + "\""; }
    internal static bool ValidGame(string path) {
        return File.Exists(Path.Combine(path,"fsgame.ltx")) && Directory.Exists(Path.Combine(path,"resources")) &&
            File.Exists(Path.Combine(path,"resources","resources.db0")) && Directory.Exists(Path.Combine(path,"levels"));
    }
    internal static string[] FindGames() {
        var found = new List<string>();
        var steam = Convert.ToString(Registry.GetValue(@"HKEY_CURRENT_USER\Software\Valve\Steam", "SteamPath", ""));
        var libraries = new List<string>{steam, Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86),"Steam")};
        foreach(var basePath in libraries.ToArray()) {
            string file=Path.Combine(basePath,"steamapps","libraryfolders.vdf");
            if(File.Exists(file)) foreach(Match match in Regex.Matches(File.ReadAllText(file), "\"path\"\\s+\"([^\"]+)\""))
                libraries.Add(match.Groups[1].Value.Replace("\\\\","\\"));
        }
        foreach(string lib in libraries.Distinct()) {
            string manifest=Path.Combine(lib,"steamapps","appmanifest_41700.acf");
            if(File.Exists(manifest)) {
                Match m=Regex.Match(File.ReadAllText(manifest),"\"installdir\"\\s+\"([^\"]+)\"");
                if(m.Success) found.Add(Path.Combine(lib,"steamapps","common",m.Groups[1].Value));
            }
            found.Add(Path.Combine(lib,"steamapps","common","Stalker Call of Pripyat"));
        }
        return found.Where(ValidGame).Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
    }
    internal static bool Running() { return Process.GetProcessesByName("xrEngine").Length != 0; }
    internal static string Runtime() {
        using(var key=RegistryKey.OpenBaseKey(RegistryHive.LocalMachine,RegistryView.Registry64).OpenSubKey(@"SOFTWARE\Khronos\OpenXR\1"))
            return key==null?"":Convert.ToString(key.GetValue("ActiveRuntime",""));
    }
    internal static void AsciiPath(string path) {
        if(path.Any(c=>c>127) || path.IndexOf('|')>=0 || path.IndexOf('"')>=0)
            throw new Exception("This alpha requires paths using Latin/ASCII characters. / Для цієї альфи оберіть шлях латиницею.");
    }
    internal static void Prepare(string root,string game) {
        if(!ValidGame(game)) throw new Exception("Select the original Steam Call of Pripyat folder (not Enhanced Edition). / Вкажіть папку оригінальної Steam-гри.");
        AsciiPath(root); AsciiPath(game);
        string data=Path.Combine(root,"data"); Directory.CreateDirectory(data);
        string template=File.ReadAllText(Path.Combine(root,"defaults","fsgame.template.ltx"));
        File.WriteAllText(Path.Combine(root,"fsgame.ltx"),template.Replace("@GAME@",game.TrimEnd('\\')+"\\").Replace("@ROOT@",root+"\\").Replace("@DATA@",data+"\\"),Encoding.ASCII);
        string cfg=Path.Combine(data,"user.ltx");
        if(!File.Exists(cfg)) File.Copy(Path.Combine(root,"defaults","user.ltx"),cfg);
        Compatibility(cfg);
        File.WriteAllText(Path.Combine(root,"game-path.txt"),game,Encoding.UTF8);
    }
    internal static void Compatibility(string file) {
        var values=new Dictionary<string,string>{
            {"renderer","renderer_r4"},{"rs_fullscreen","off"},{"vid_window_mode","st_opt_windowed_borderless"},
            {"r3_msaa","st_opt_off"},{"r3_msaa_alphatest","st_opt_off"},{"r2_ssao_hdao","off"},{"r2_ssao_hbao","on"},{"r2_ssao_mode","hbao"}};
        var lines=File.ReadAllLines(file).Where(l=>!values.ContainsKey(l.Trim().Split(new[]{' ','\t'},StringSplitOptions.RemoveEmptyEntries).FirstOrDefault()??"")).ToList();
        lines.AddRange(values.Select(p=>p.Key+" "+p.Value)); File.WriteAllLines(file,lines,Encoding.ASCII);
    }
    internal static void Open(string path) { Process.Start(new ProcessStartInfo(path){UseShellExecute=true}); }
}
