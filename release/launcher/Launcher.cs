using System;
using System.IO;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;

internal sealed class Launcher : Form {
    readonly Label status=new Label(); readonly Button play=new Button(); readonly ComboBox language=new ComboBox();
    bool Uk {get {return language.SelectedIndex==1;}}
    string T(string en,string uk) {return Uk?uk:en;}
    [STAThread] static int Main(string[] args) {
        if(Array.IndexOf(args,"--self-check")>=0) {
            try { foreach(string f in new[]{"bin/xrEngine.exe","bin/xrGame.dll","bin/copvr_bridge.dll","bin/openxr_loader.dll","defaults/user.ltx","defaults/fsgame.template.ltx","docs/USER-GUIDE.en.md","docs/USER-GUIDE.uk.md"})
                if(!File.Exists(Path.Combine(Product.Root,f)))throw new Exception(f);return 0;
            }catch{return 1;}
        }
        Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);Application.Run(new Launcher());return 0;
    }
    Launcher() {
        AutoScaleMode=AutoScaleMode.None;
        Font=new Font("Segoe UI",10);
        Text="Call of Pripyat VR — "+Product.Version; ClientSize=new Size(760,460); MinimumSize=Size;
        string icon=Path.Combine(Product.Root,"CoPVR.ico");if(File.Exists(icon))Icon=new Icon(icon);
        StartPosition=FormStartPosition.CenterScreen;
        var title=new Label{Text="CALL OF PRIPYAT VR",Font=new Font("Segoe UI",20,FontStyle.Bold),Location=new Point(24,20),AutoSize=true};Controls.Add(title);
        language.Items.AddRange(new object[]{"English","Українська"});language.DropDownStyle=ComboBoxStyle.DropDownList;language.SetBounds(560,28,175,30);Controls.Add(language);
        status.SetBounds(24,85,710,145);Controls.Add(status);
        play.SetBounds(24,235,710,48);play.Click+=(s,e)=>StartGame();Controls.Add(play);
        AddButton(24,300,345,()=>T("User guide","Керування та можливості"),()=>Product.Open(Path.Combine(Product.Root,"docs",Uk?"USER-GUIDE.uk.html":"USER-GUIDE.en.html")));
        AddButton(389,300,345,()=>T("Saves, settings and logs","Збереження, конфіг та журнали"),()=>Product.Open(Path.Combine(Product.Root,"data")));
        AddButton(24,350,345,()=>T("Change game folder","Змінити папку гри"),ChangeGame);
        AddButton(389,350,345,()=>T("Reset graphics to release defaults","Відновити релізну графіку"),Reset);
        Controls.Add(new Label{Text="Unofficial free alpha • Original Steam game required • Quest 2 / Touch / USB Link",Location=new Point(24,420),AutoSize=true,Font=new Font("Segoe UI",8)});
        language.SelectedIndexChanged+=(s,e)=>RefreshText(); language.SelectedIndex=System.Globalization.CultureInfo.CurrentUICulture.TwoLetterISOLanguageName=="uk"?1:0;
        using(var graphics=CreateGraphics()) {float factor=graphics.DpiX/96f;if(factor!=1f)Scale(new SizeF(factor,factor));}
        MinimumSize=Size;
    }
    void AddButton(int x,int y,int width,Func<string> text,Action action) {
        var b=new Button{Location=new Point(x,y),Size=new Size(width,36),Tag=text};b.Click+=(s,e)=>{try{action();RefreshText();}catch(Exception ex){MessageBox.Show(ex.Message,Text);}};Controls.Add(b);
    }
    string Game {get {string p=Path.Combine(Product.Root,"game-path.txt");return File.Exists(p)?File.ReadAllText(p).Trim():"";}}
    void RefreshText() {
        play.Text=T("Play in VR","Запустити у VR");
        string runtime=Product.Runtime();
        status.Text=T("Game: ","Гра: ")+Game+"\n"+T("OpenXR runtime: ","Середовище OpenXR: ")+(File.Exists(runtime)?Path.GetFileName(runtime):T("not configured","не налаштовано"))+"\n\n"+T("Open Quest Link and put on the headset before Play. Stand upright and click the right stick to calibrate in-game.","Перед запуском відкрийте Quest Link і надягніть гарнітуру. У грі станьте рівно та натисніть правий стік для калібрування.");
        foreach(Control c in Controls) if(c.Tag is Func<string>)c.Text=((Func<string>)c.Tag)();
    }
    void ChangeGame() {
        if(Product.Running())throw new Exception(T("Close the game first.","Спершу закрийте гру."));
        using(var dialog=new FolderBrowserDialog{Description=T("Original Steam Call of Pripyat folder","Папка оригінальної Steam Call of Pripyat"),SelectedPath=Game})
            if(dialog.ShowDialog()==DialogResult.OK)Product.Prepare(Product.Root,dialog.SelectedPath);
    }
    void Reset() {
        if(Product.Running())throw new Exception(T("Close the game first.","Спершу закрийте гру."));
        if(MessageBox.Show(T("Back up current settings and restore release graphics?","Зберегти резервну копію налаштувань та відновити релізну графіку?"),Text,MessageBoxButtons.YesNo)!=DialogResult.Yes)return;
        string cfg=Path.Combine(Product.Root,"data","user.ltx");if(File.Exists(cfg))File.Copy(cfg,cfg+"."+DateTime.Now.ToString("yyyyMMdd-HHmmssfff")+".bak");
        File.Copy(Path.Combine(Product.Root,"defaults","user.ltx"),cfg,true);Product.Compatibility(cfg);
    }
    void StartGame() {
        try {
            if(Product.Running())throw new Exception(T("An xrEngine process is already running.","Процес xrEngine вже запущений."));
            if(!File.Exists(Product.Runtime()))throw new Exception(T("No active 64-bit OpenXR runtime. Select Meta Quest Link as the OpenXR runtime in the Meta PC app.","Немає активного 64-бітного OpenXR. Виберіть Meta Quest Link як середовище OpenXR у програмі Meta на ПК."));
            Product.Prepare(Product.Root,Game);
            var p=Process.Start(new ProcessStartInfo(Path.Combine(Product.Root,"bin","xrEngine.exe"),"-fsltx fsgame.ltx -noprefetch -copvr -nointro -r4 -copvr_eye 2080x2096 -copvr_shadow_cache -copvr_shadow_tiles -copvr_shadow_pipeline -copvr_skip_base_camera_work"){WorkingDirectory=Product.Root,UseShellExecute=false});
            play.Enabled=false;p.Exited+=(s,e)=>{if(!IsDisposed)BeginInvoke(new Action(()=>play.Enabled=true));};p.EnableRaisingEvents=true;
        }catch(Exception ex){MessageBox.Show(ex.Message,Text,MessageBoxButtons.OK,MessageBoxIcon.Error);}
    }
}
