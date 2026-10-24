using System;
using System.IO;
using System.IO.Compression;
using System.Reflection;
using System.Drawing;
using System.Linq;
using System.Threading.Tasks;
using System.Security.Cryptography;
using System.Windows.Forms;
using Microsoft.Win32;

internal sealed class Setup : Form {
    readonly TextBox game=new TextBox(),destination=new TextBox();readonly Button install=new Button();readonly Label status=new Label();readonly CheckBox shortcut=new CheckBox();
    [STAThread] static int Main(string[] args) {
        if(args.Length==3&&args[0]=="--verify-extract") {try{Extract(args[1],args[2]);return 0;}catch(Exception e){File.WriteAllText(args[1]+".error.txt",e.ToString());return 1;}}
        Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);Application.Run(new Setup());return 0;
    }
    Setup() {
        AutoScaleMode=AutoScaleMode.None;Font=new Font("Segoe UI",10);
        Text="Call of Pripyat VR — Setup / Встановлення — "+Product.Version;ClientSize=new Size(710,440);FormBorderStyle=FormBorderStyle.FixedDialog;MaximizeBox=false;StartPosition=FormStartPosition.CenterScreen;
        AddLabel("Call of Pripyat VR • Unofficial free alpha / Неофіційна безкоштовна альфа",20,18,670,28);
        AddLabel("Requires original Steam CoP 1.6.02, Windows x64, Quest Link + Touch.\nПотрібні оригінальна Steam-гра 1.6.02, Windows x64, Quest Link + Touch.",20,55,670,48);
        AddLabel("Original game folder / Папка оригінальної гри",20,120,650,25);game.SetBounds(20,148,570,28);game.Text=Product.FindGames().FirstOrDefault()??"";Controls.Add(game);Browse(game,600,148);
        AddLabel("Install mod separately / Окрема папка мода (файли гри не змінюються)",20,190,670,25);destination.SetBounds(20,220,570,28);destination.Text=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"Programs","CoPVR",Product.Version);Controls.Add(destination);Browse(destination,600,220);
        shortcut.Text="Desktop shortcut / Ярлик на робочому столі";shortcut.Checked=true;shortcut.SetBounds(20,267,650,28);Controls.Add(shortcut);
        status.SetBounds(20,306,670,50);Controls.Add(status);install.Text="Install / Встановити";install.SetBounds(20,370,670,44);install.Click+=async(s,e)=>await Install();Controls.Add(install);
        using(var graphics=CreateGraphics()) {float factor=graphics.DpiX/96f;if(factor!=1f)Scale(new SizeF(factor,factor));}
    }
    void AddLabel(string text,int x,int y,int w,int h){Controls.Add(new Label{Text=text,Location=new Point(x,y),Size=new Size(w,h)});}
    void Browse(TextBox field,int x,int y){var b=new Button{Text="…",Location=new Point(x,y),Size=new Size(70,28)};b.Click+=(s,e)=>{using(var d=new FolderBrowserDialog{SelectedPath=field.Text})if(d.ShowDialog()==DialogResult.OK)field.Text=d.SelectedPath;};Controls.Add(b);}
    internal static void Extract(string root,string game) {
        root=Path.GetFullPath(root).TrimEnd('\\');game=Path.GetFullPath(game).TrimEnd('\\');Product.AsciiPath(root);Product.AsciiPath(game);
        if(!Product.ValidGame(game))throw new Exception("Original game archives not found.");
        if(root.Equals(game,StringComparison.OrdinalIgnoreCase)||root.StartsWith(game+"\\",StringComparison.OrdinalIgnoreCase)||game.StartsWith(root+"\\",StringComparison.OrdinalIgnoreCase))throw new Exception("Install outside the original game folder.");
        if(Directory.Exists(root)&&Directory.EnumerateFileSystemEntries(root).Any())throw new Exception("Choose a new empty folder. / Оберіть нову порожню папку.");
        Directory.CreateDirectory(root);
        using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("payload.zip"))
        using(var zip=new ZipArchive(stream,ZipArchiveMode.Read)) foreach(var entry in zip.Entries) {
            string path=Path.GetFullPath(Path.Combine(root,entry.FullName.Replace('/',Path.DirectorySeparatorChar)));
            if(!path.StartsWith(root+"\\",StringComparison.OrdinalIgnoreCase))throw new Exception("Invalid package path.");
            if(entry.Name.Length==0){Directory.CreateDirectory(path);continue;}
            Directory.CreateDirectory(Path.GetDirectoryName(path));using(var input=entry.Open())using(var output=File.Create(path))input.CopyTo(output);
        }
        foreach(string line in File.ReadAllLines(Path.Combine(root,"integrity.sha256"))) {
            string relative=line.Substring(66),path=Path.GetFullPath(Path.Combine(root,relative));
            if(!path.StartsWith(root+"\\",StringComparison.OrdinalIgnoreCase))throw new Exception("Invalid integrity path.");
            using(var sha=SHA256.Create())using(var file=File.OpenRead(path))
                if(BitConverter.ToString(sha.ComputeHash(file)).Replace("-","").ToLowerInvariant()!=line.Substring(0,64))throw new Exception("Package integrity failed: "+relative);
        }
        Product.Prepare(root,game);
        File.WriteAllText(Path.Combine(root,"copvr-install.marker"),Product.Version);
    }
    async Task Install() {
        install.Enabled=false;string root=destination.Text.Trim(),source=game.Text.Trim();
        try {
            if(Product.Running())throw new Exception("Close xrEngine first. / Закрийте xrEngine.");
            status.Text="Installing… / Встановлення…";await Task.Run(()=>Extract(root,source));
            root=Path.GetFullPath(root);
            if(shortcut.Checked) {
                Type type=Type.GetTypeFromProgID("WScript.Shell");object shell=Activator.CreateInstance(type);
                object link=type.InvokeMember("CreateShortcut",BindingFlags.InvokeMethod,null,shell,new object[]{Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory),"Call of Pripyat VR.lnk")});
                Type lt=link.GetType();lt.InvokeMember("TargetPath",BindingFlags.SetProperty,null,link,new object[]{Path.Combine(root,"CoPVR.exe")});lt.InvokeMember("WorkingDirectory",BindingFlags.SetProperty,null,link,new object[]{root});lt.InvokeMember("Save",BindingFlags.InvokeMethod,null,link,null);
            }
            using(var key=Registry.CurrentUser.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Uninstall\CoPVR-"+Product.Version)) {
                key.SetValue("DisplayName","Call of Pripyat VR "+Product.Version);key.SetValue("DisplayVersion",Product.Version);key.SetValue("InstallLocation",root);key.SetValue("DisplayIcon",Path.Combine(root,"CoPVR.exe"));
                key.SetValue("UninstallString","powershell.exe -NoProfile -ExecutionPolicy Bypass -File "+Product.Q(Path.Combine(root,"Uninstall.ps1")));key.SetValue("NoModify",1);key.SetValue("NoRepair",1);
            }
            status.Text="Installed. Game was NOT launched. / Встановлено. Гру НЕ запущено.";install.Text="Open launcher / Відкрити лаунчер";
            install.Enabled=false;
            var open=new Button{Text="Open launcher / Відкрити лаунчер",Location=install.Location,Size=install.Size};open.Click+=(s,e)=>Product.Open(Path.Combine(root,"CoPVR.exe"));Controls.Remove(install);Controls.Add(open);
        }catch(Exception ex){status.Text="Installation failed / Помилка встановлення";MessageBox.Show(ex.Message,Text,MessageBoxButtons.OK,MessageBoxIcon.Error);install.Enabled=true;}
    }
}
