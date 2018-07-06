package main

import (
	"fmt"
	"os"
	"os/exec"
	//	"unicode/utf8"
	//"path/filepath"
	"bufio"
	"io"
	"io/ioutil"

	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"

	//	"github.com/djimenez/iconv-go"
	"github.com/mattn/go-gtk/gdk"
	"github.com/mattn/go-gtk/glib"
	"github.com/mattn/go-gtk/gtk"
)

func uniq(strings []string) (ret []string) {
	return
}

func authors() []string {
	if b, err := exec.Command("git", "log").Output(); err == nil {
		lines := strings.Split(string(b), "\n")

		var a []string
		r := regexp.MustCompile(`^Author:\s*([^ <]+).*$`)
		for _, e := range lines {
			ms := r.FindStringSubmatch(e)
			if ms == nil {
				continue
			}
			a = append(a, ms[1])
		}
		sort.Strings(a)
		var p string
		lines = []string{}
		for _, e := range a {
			if p == e {
				continue
			}
			lines = append(lines, e)
			p = e
		}
		return lines
	}
	return []string{"Yasuhiro Matsumoto <mattn.jp@gmail.com>"}
}

// define
var textview_file1 *gtk.TextView
var textview_file2 *gtk.TextView
var textview_file3 *gtk.TextView

var tv_btn_left *gtk.TextView
var tv_btn_l_right *gtk.TextView
var tv_oled_light *gtk.TextView
var tv_oled_dark *gtk.TextView
var tv_fw_earse *gtk.TextView

var textview_output *gtk.TextView
var btn_load *gtk.Button
var btn_start *gtk.Button
var buttons *gtk.HBox

var framebox1 *gtk.VBox
var framebox2 *gtk.VBox
var framebox_text *gtk.HBox

var progressbar *gtk.ProgressBar

var isTestMode bool = true

func ui_show() {
	gtk.Init(&os.Args)

	window := gtk.NewWindow(gtk.WINDOW_TOPLEVEL)
	window.SetPosition(gtk.WIN_POS_CENTER)
	window.SetTitle(" ")
	window.SetIconName("gtk-dialog-info")
	window.Connect("destroy", func(ctx *glib.CallbackContext) {
		fmt.Println("got destroy!", ctx.Data().(string))
		gtk.MainQuit()
	}, "foo")

	var windos_w int = 700
	var windos_h int = 600

	currentPath = getCurrDir()

	//--------------------------------------------------------
	// GtkVBox
	//--------------------------------------------------------
	vbox := gtk.NewVBox(false, 1)
	//--------------------------------------------------------
	// GtkVPaned
	//--------------------------------------------------------
	vpaned := gtk.NewVPaned()

	//--------------------------------------------------------
	// GtkFrame
	//--------------------------------------------------------
	frame1 := gtk.NewFrame("")
	framebox1 = gtk.NewVBox(false, 1)

	frame2 := gtk.NewFrame("")
	framebox2 = gtk.NewVBox(false, 1)

	//-------------------------------framebox1-------start------------------------
	label := gtk.NewLabel("JusetSafe 固件下载工具 v1.0 ")
	label.ModifyFontEasy("Sans 18")
	color := gdk.NewColor("red")
	fmt.Println("color:", color)
	label.ModifyText(gtk.STATE_NORMAL, color)
	label.ModifyBG(gtk.STATE_NORMAL, color)

	framebox1.PackStart(label, false, true, 0)

	// GtkVSeparator
	vsep := gtk.NewVSeparator()
	vsep.Activate()
	vsep.SetSizeRequest(10, 10)
	framebox1.PackStart(vsep, false, false, 0)

	//--------------------------------------------------------
	// GtkHBox
	//--------------------------------------------------------
	buttons = gtk.NewHBox(false, 1)

	//--------------------------------------------------------
	// GtkButton
	//--------------------------------------------------------
	btn_load = gtk.NewButtonWithLabel("加载固件")
	btn_load.SetSizeRequest(50, 50)
	btn_load.ModifyFontEasy("DejaVu Serif 20")
	buttons.Add(btn_load)

	//btn_start = gtk.NewButtonWithLabel("开始")
	//btn_start.SetSensitive(false)

	//btn_start.SetSizeRequest(50, 50)
	//buttons.Add(btn_start)

	framebox1.PackStart(buttons, false, false, 0)

	// GtkVSeparator
	vsep0 := gtk.NewVSeparator()
	vsep0.Activate()
	vsep0.SetSizeRequest(5, 5)
	framebox1.PackStart(vsep0, false, false, 0)

	// testview
	textview_file1 = gtk.NewTextView()
	textview_file2 = gtk.NewTextView()
	textview_file3 = gtk.NewTextView()

	textview_file1.SetCanFocus(false)
	textview_file2.SetCanFocus(false)
	textview_file3.SetCanFocus(false)

	textview_file1.ModifyFontEasy("Sans 15")
	textview_file2.ModifyFontEasy("Sans 15")
	textview_file3.ModifyFontEasy("Sans 15")

	framebox1.PackStart(textview_file1, false, false, 0)
	framebox1.PackStart(textview_file2, false, false, 0)
	framebox1.PackStart(textview_file3, false, false, 0)
	// testview ---- for test

	tv_btn_left = gtk.NewTextView()
	tv_btn_l_right = gtk.NewTextView()
	tv_oled_light = gtk.NewTextView()
	tv_oled_dark = gtk.NewTextView()
	tv_fw_earse = gtk.NewTextView()

	tv_btn_left.SetCanFocus(false)
	tv_btn_l_right.SetCanFocus(false)
	tv_oled_light.SetCanFocus(false)
	tv_oled_dark.SetCanFocus(false)
	tv_fw_earse.SetCanFocus(false)

	tv_btn_left.ModifyFontEasy("Sans 12")
	tv_btn_l_right.ModifyFontEasy("Sans 12")
	tv_oled_light.ModifyFontEasy("Sans 12")
	tv_oled_dark.ModifyFontEasy("Sans 12")
	tv_fw_earse.ModifyFontEasy("Sans 12")

	framebox_text = gtk.NewHBox(false, 1)
	tv_btn_left.GetBuffer().SetText("")
	test_label := gtk.NewLabel(" ")
	test2_label := gtk.NewLabel(" ")

	framebox_text.PackStart(test_label, false, false, 0)
	framebox_text.PackStart(test2_label, false, false, 0)
	//framebox_text.PackStart(tv_oled_light, false, false, 0)
	//framebox_text.PackStart(tv_oled_dark, false, false, 0)
	//framebox_text.PackStart(tv_fw_earse, false, false, 0)
	framebox1.PackStart(framebox_text, false, false, 0)
	// GtkVSeparator
	//vsep1 := gtk.NewVSeparator()
	//vsep1.Activate()
	//vsep1.SetSizeRequest(1, 1)
	//framebox1.PackStart(vsep1, false, false, 0)

	// progressbar
	progressbarBox := gtk.NewFixed()
	progressbar = gtk.NewProgressBar()
	progressbar.SetSizeRequest(windos_w, 90)
	progressbar.SetOrientation(gtk.PROGRESS_LEFT_TO_RIGHT)
	progressbar.SetFraction(0)
	progressbar.SetText("(0%)")
	progressbar.ModifyFontEasy("Sans 15")
	progressbarBox.Add(progressbar)
	fmt.Println("isTestMode:", isTestMode)
	if isTestMode == true {
		framebox1.Add(progressbarBox)
	}
	//-------------------------------framebox1-----end--------------------------

	//-------------------------------framebox2----start---------------------------
	//--------------------------------------------------------
	// GtkTextView ScrolledWindow
	//--------------------------------------------------------
	swin := gtk.NewScrolledWindow(nil, nil)
	swin.SetPolicy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
	swin.SetSizeRequest(500, 1000)
	swin.SetShadowType(gtk.SHADOW_ETCHED_OUT)
	textview_output = gtk.NewTextView()
	swin.Add(textview_output)
	framebox2.Add(swin)
	//-------------------------------framebox2------end-------------------------

	//-------------------------------register event -------------------------------
	btn_load.Clicked(func() {
		fmt.Println("mBinCfgFilePath:", mBinCfgFilePath)
		if mBinCfgFilePath == "" {
			//--------------------------------------------------------
			// GtkFileChooserDialog
			//--------------------------------------------------------
			filechooserdialog := gtk.NewFileChooserDialog(
				"Choose File...",
				btn_load.GetTopLevelAsWindow(),
				gtk.FILE_CHOOSER_ACTION_OPEN,
				gtk.STOCK_OK,
				gtk.RESPONSE_ACCEPT)

			filter := gtk.NewFileFilter()
			filter.AddPattern("*.txt")
			filechooserdialog.AddFilter(filter)
			filechooserdialog.Response(func() {
				parsefilepath(textview_file1, textview_file2, textview_file3, filechooserdialog.GetFilename())
				filechooserdialog.Destroy()
			})
			filechooserdialog.Run()
		} else {
			fmt.Println("step 9")
			textview_output.ModifyFontEasy("Sans 12")
			textview_output.GetBuffer().SetText(" ")
			setOutputTextString(" ************************[START DOWNLOAD]************************\n\n")
			fmt.Println("step 10")
			go download_sw()
			buttons.SetSensitive(false)
			fmt.Println("step 11")
		}
	})

	//-------------------------------register event-------------------------------

	frame1.Add(framebox1)
	vpaned.Pack1(frame1, false, false)

	frame2.Add(framebox2)
	vpaned.Pack2(frame2, false, false)

	vbox.Add(vpaned)
	window.Add(vbox)

	window.SetSizeRequest(windos_w, windos_h)
	window.ShowAll()

	gdk.ThreadsEnter()
	gtk.Main()
}

func download_sw() {
	var i int = 0

	for {
		i = i + 1
		if i == 2 {
			break
		}
		fmt.Println(" -----NO： %d", i)
		stlink_download()
	}

	buttons.SetSensitive(true)
}

func ModeSelectHideUI(test bool) {
	isTestMode = test
	if test == true {
		textview_file1.Hide()
		textview_file2.Hide()
		textview_file3.Hide()
		progressbar.Hide()
		framebox2.Hide()
		btn_load.SetSensitive(false)
	} else {
		textview_file1.Show()
		textview_file2.Show()
		textview_file3.Show()
		progressbar.Show()
		framebox2.Show()
		btn_load.SetSensitive(true)
	}

}

func is_ftm_download() bool {
	var i int = 0
	for {
		if Binfile[i].name == "" {
			return false
		}
		if strings.Contains(Binfile[i].name, "ftm") {
			return true
		}
		i = i + 1
	}
	return false
}

func setOutputTextString(s string) {
	var start gtk.TextIter
	buffer := textview_output.GetBuffer()
	textview_output.GetBuffer().GetStartIter(&start)
	buffer.Insert(&start, s)
}

func stlink_download() {
	var ret int = 0
	var cmd1, cmd2, cmd3 string

	if is_ftm_download() {
		cmd1 = "b"
		cmd2 = "ftm-m"
		cmd3 = "ftm-f"
	} else {
		cmd1 = "b"
		cmd2 = "m"
		cmd3 = "f"
	}

	fmt.Println("step 1")

	exec_stlink_download("reset-stlink")

	glib.IdleAdd(func() bool {
		progressbar.SetFraction(0)
		progressbar.SetText("Programming bootloader..")
		return false
	})

	fmt.Println("step 2")
	ret = exec_stlink_download(cmd1)
	if ret == 0 {
		glib.IdleAdd(func() bool {
			progressbar.SetFraction(0.33)
			return false
		})
	} else {
		goto exit
	}

	fmt.Println("step 3")
	glib.IdleAdd(func() bool {
		progressbar.SetText("Programming metadata..")
		return false
	})

	ret = exec_stlink_download(cmd2)
	if ret == 0 {
		glib.IdleAdd(func() bool {
			progressbar.SetFraction(0.7)
			return false
		})
	} else {
		goto exit
	}
	fmt.Println("step 4")

	glib.IdleAdd(func() bool {
		progressbar.SetText("Programming firmware..")
		return false
	})

	ret = exec_stlink_download(cmd3)
	if ret == 0 {
		glib.IdleAdd(func() bool {
			progressbar.SetFraction(1)
			progressbar.SetText("Success")
			setOutputTextString(" ************************[DOWNLOAD END]************************\n")
			return false
		})
	} else {
		goto exit
	}

	fmt.Println("step 5")
	return

exit:

	glib.IdleAdd(func() bool {
		progressbar.SetText("FAILED!")
		buttons.SetSensitive(true)
		return false
	})
}

func exec_shell_sh(action string) string {
	var script string
	var path string
	fmt.Println("step 6")
	if runtime.GOOS == "linux" {
		script = "/" + "download.sh"
		path = strings.Replace(mBinCfgFilePath, "/map.txt", "", -1)
	} else {
		script = "\\" + "download.bat"
		path = strings.Replace(mBinCfgFilePath, "\\map.txt", "", -1)
	}

	cmd := exec.Command(currentPath+script, path, action)
	fmt.Println("cmd:", cmd.Args)
	stdout, err := cmd.StdoutPipe()
	cmd.StdoutPipe()
	cmd.Start()
	content, err := ioutil.ReadAll(stdout)
	fmt.Println("step 7")
	if err != nil {
		fmt.Println(err)
		return "err"
	}

	outstr0 := string(content)
	fmt.Println("####outstr#####:", outstr0)
	fmt.Println("####outstr#### end\n")
	//outstr, err := iconv.ConvertString(outstr0, "ebcdic-us", "utf-8")
	//iconv.ConvertString(outstr0, "GB2312", "utf-8")
	return outstr0
}

func exec_stlink_download(action string) int {
	log := exec_shell_sh(action)
	fmt.Println(log)

	var keyLogSuccess, keyLogSuccess2, keyLogSuccess3, keyLogNoSTlink string

	if runtime.GOOS == "linux" {
		glib.IdleAdd(func() bool {
			setOutputTextString(log)
			return false
		})
		keyLogSuccess = "Flash written and verified! jolly good!"
		keyLogNoSTlink = "Couldn't find any ST-Link"
	} else {
		glib.IdleAdd(func() bool {
			outstr1 := substring(log, 0, 500)
			setOutputTextString(outstr1 + "\n")
			return false
		})
		keyLogSuccess = "This file is already downloaded"
		keyLogSuccess2 = "Programming Complete"
		keyLogSuccess3 = "Verification...OK"

		keyLogNoSTlink = "No ST-LINK detected!"
	}

	var ret bool
	if log != "" {
		ret = strings.Contains(log, keyLogSuccess) || strings.Contains(log, keyLogSuccess2) || strings.Contains(log, keyLogSuccess3)
		if ret == true {
			return 0
		}
		ret = strings.Contains(log, keyLogNoSTlink)
		if ret == true {
			glib.IdleAdd(func() bool {
				setOutputTextString(keyLogNoSTlink + "\n")
				return false
			})
			return -2
		}
	}

	return -1
}

func parsefilepath(f1 *gtk.TextView, f2 *gtk.TextView, f3 *gtk.TextView, f string) string {
	if f == "" {
		return ""
	}

	btn_load.SetLabel("开始烧录")
	//btn_start.SetSensitive(true)

	mBinCfgFilePath = f
	ReadLine(f)
	var path string
	if runtime.GOOS == "linux" {
		path = strings.Replace(f, "map.txt", "", -1)
	} else {
		path = strings.Replace(f, "map.txt", "", -1)
	}

	f1.GetBuffer().SetText(path + Binfile[0].name + " " + Binfile[0].to)
	f2.GetBuffer().SetText(path + Binfile[1].name + " " + Binfile[1].to)
	f3.GetBuffer().SetText(path + Binfile[2].name + " " + Binfile[2].to)

	fmt.Println("mBinCfgFilePath:", mBinCfgFilePath)
	fmt.Println("f:", f)
	return path
}

type Binfile_t struct {
	name string
	to   string
}

var mBinCfgFilePath string
var currentPath string

var Binfile [4]Binfile_t

func getCurrDir() string {
	d, e := filepath.Abs(filepath.Dir(os.Args[0]))
	if e != nil {
		fmt.Println("err")
	}
	//return strings.Replace(d, "\\", "/", -1)
	return d
}

func substring(source string, start int, end int) string {
	var substring = ""
	var pos = 0
	for _, c := range source {
		if pos < start {
			pos++
			continue
		}
		if pos >= end {
			break
		}
		pos++
		substring += string(c)
	}
	return substring
}

func parseLine(l string, i int) {
	var s, s1 string
	idx := strings.Index(l, ":")
	s = substring(l, 0, idx)
	if idx > 0 {
		s1 = substring(l, idx+1, idx+1+10)
		Binfile[i].name = s
		Binfile[i].to = s1
	}
	fmt.Println(i, s, s1)
}

func ReadLine(fileName string) error {
	f, err := os.Open(fileName)
	if err != nil {
		return err
	}

	buf := bufio.NewReader(f)
	var i int = 0

	for {
		line, err := buf.ReadString('\n')
		line = strings.TrimSpace(line)
		if i < 3 {
			parseLine(line, i)
			i = i + 1
		}
		if err != nil {
			if err == io.EOF {
				break
			}
			return err
		}
	}
	return nil
}
