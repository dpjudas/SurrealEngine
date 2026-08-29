#pragma once

static const char* theme_default_stylesheet = R"xxxx(

.dark {
	--textarea-text: #ffffff;
	--textarea-bg: #111111;

	--primary-50: #545454;
	--primary-100: #505050;
	--primary-200: #4a4a4a;
	--primary-300: #484848;
	--primary-400: #404040;
	--primary-500: #383838;
	--primary-600: #202020;
	--primary-700: #181818;
	--primary-800: #101010;
	--primary-900: #080808;
	--primary-950: #000000;

	--primary-text-50: #ffffff;
	--primary-text-100: #ffffff;
	--primary-text-200: #ffffff;
	--primary-text-300: #ffffff;
	--primary-text-400: #ffffff;
	--primary-text-500: #ffffff;
	--primary-text-600: #ffffff;
	--primary-text-700: #ffffff;
	--primary-text-800: #ffffff;
	--primary-text-900: #ffffff;
	--primary-text-950: #ffffff;

	--secondary-50: #f9e3e0;
	--secondary-100: #f3c8c0;
	--secondary-200: #edaca1;
	--secondary-300: #e79181;
	--secondary-400: #e17562;
	--secondary-500: #db5a42;
	--secondary-600: #c83e26;
	--secondary-700: #a0321e;
	--secondary-800: #782517;
	--secondary-900: #50190f;
	--secondary-950: #280c08;

	--secondary-text-50: #000000;
	--secondary-text-100: #000000;
	--secondary-text-200: #000000;
	--secondary-text-300: #ffffff;
	--secondary-text-400: #ffffff;
	--secondary-text-500: #ffffff;
	--secondary-text-600: #ffffff;
	--secondary-text-700: #ffffff;
	--secondary-text-800: #ffffff;
	--secondary-text-900: #ffffff;
	--secondary-text-950: #ffffff;

	--tertiary-50: #f6d0c6;
	--tertiary-100: #eda18c;
	--tertiary-200: #e47253;
	--tertiary-300: #d44821;
	--tertiary-400: #9a3418;
	--tertiary-500: #61210f;
	--tertiary-600: #511b0d;
	--tertiary-700: #41160a;
	--tertiary-800: #311008;
	--tertiary-900: #200b05;
	--tertiary-950: #100503;

	--tertiary-text-50: #000000;
	--tertiary-text-100: #000000;
	--tertiary-text-200: #ffffff;
	--tertiary-text-300: #ffffff;
	--tertiary-text-400: #ffffff;
	--tertiary-text-500: #ffffff;
	--tertiary-text-600: #ffffff;
	--tertiary-text-700: #ffffff;
	--tertiary-text-800: #ffffff;
	--tertiary-text-900: #ffffff;
	--tertiary-text-950: #ffffff;
}

.bordeaux {
	--textarea-text: #000000;
	--textarea-bg: #eeeeee;

	--primary-50: #fdbfce;
	--primary-100: #fa7f9e;
	--primary-200: #f83f6d;
	--primary-300: #ec0941;
	--primary-400: #ac0630;
	--primary-500: #6a041d;
	--primary-600: #5a0319;
	--primary-700: #480314;
	--primary-800: #36020f;
	--primary-900: #24010a;
	--primary-950: #120105;

	--primary-text-50: #000000;
	--primary-text-100: #ffffff;
	--primary-text-200: #ffffff;
	--primary-text-300: #ffffff;
	--primary-text-400: #ffffff;
	--primary-text-500: #ffffff;
	--primary-text-600: #ffffff;
	--primary-text-700: #ffffff;
	--primary-text-800: #ffffff;
	--primary-text-900: #ffffff;
	--primary-text-950: #ffffff;

	--secondary-50: #fdf3e0;
	--secondary-100: #fce8c0;
	--secondary-200: #fadca1;
	--secondary-300: #f8d181;
	--secondary-400: #f7c562;
	--secondary-500: #f5b841;
	--secondary-600: #f2a711;
	--secondary-700: #c5870a;
	--secondary-800: #946508;
	--secondary-900: #634305;
	--secondary-950: #312203;

	--secondary-text-50: #000000;
	--secondary-text-100: #000000;
	--secondary-text-200: #000000;
	--secondary-text-300: #000000;
	--secondary-text-400: #000000;
	--secondary-text-500: #000000;
	--secondary-text-600: #000000;
	--secondary-text-700: #ffffff;
	--secondary-text-800: #ffffff;
	--secondary-text-900: #ffffff;
	--secondary-text-950: #ffffff;

	--tertiary-50: #fdffe2;
	--tertiary-100: #fbffc5;
	--tertiary-200: #f9ffa8;
	--tertiary-300: #f7ff8b;
	--tertiary-400: #f5ff6f;
	--tertiary-500: #f4ff52;
	--tertiary-600: #f0ff1a;
	--tertiary-700: #d1e000;
	--tertiary-800: #9da800;
	--tertiary-900: #697000;
	--tertiary-950: #343800;

	--tertiary-text-50: #000000;
	--tertiary-text-100: #000000;
	--tertiary-text-200: #000000;
	--tertiary-text-300: #000000;
	--tertiary-text-400: #000000;
	--tertiary-text-500: #000000;
	--tertiary-text-600: #000000;
	--tertiary-text-700: #000000;
	--tertiary-text-800: #ffffff;
	--tertiary-text-900: #ffffff;
	--tertiary-text-950: #ffffff;
}

.light {
	--textarea-text: #000000;
	--textarea-bg: #eeeeee;

	--primary-50: #f4f4f4;
	--primary-100: #f0f0f0;
	--primary-200: #eaeaea;
	--primary-300: #e8e8e8;
	--primary-400: #e0e0e0;
	--primary-500: #d8d8d8;
	--primary-600: #c0c0c0;
	--primary-700: #b8b8b8;
	--primary-800: #b0b0b0;
	--primary-900: #a8a8a8;
	--primary-950: #a0a0a0;

	--primary-text-50: #000000;
	--primary-text-100: #000000;
	--primary-text-200: #000000;
	--primary-text-300: #000000;
	--primary-text-400: #000000;
	--primary-text-500: #000000;
	--primary-text-600: #000000;
	--primary-text-700: #000000;
	--primary-text-800: #ffffff;
	--primary-text-900: #ffffff;
	--primary-text-950: #ffffff;

	--secondary-50: #e5edef;
	--secondary-100: #cadbdf;
	--secondary-200: #b0c8cf;
	--secondary-300: #96b6be;
	--secondary-400: #7ba4ae;
	--secondary-500: #62929e;
	--secondary-600: #517a84;
	--secondary-700: #416169;
	--secondary-800: #30494f;
	--secondary-900: #203135;
	--secondary-950: #10181a;

	--secondary-text-50: #000000;
	--secondary-text-100: #000000;
	--secondary-text-200: #000000;
	--secondary-text-300: #000000;
	--secondary-text-400: #ffffff;
	--secondary-text-500: #ffffff;
	--secondary-text-600: #ffffff;
	--secondary-text-700: #ffffff;
	--secondary-text-800: #ffffff;
	--secondary-text-900: #ffffff;
	--secondary-text-950: #ffffff;

	--tertiary-50: #dfe8ec;
	--tertiary-100: #bed0d8;
	--tertiary-200: #9eb9c5;
	--tertiary-300: #7da2b1;
	--tertiary-400: #5e8a9d;
	--tertiary-500: #4a6d7c;
	--tertiary-600: #3e5b68;
	--tertiary-700: #324953;
	--tertiary-800: #25373e;
	--tertiary-900: #192429;
	--tertiary-950: #0c1215;

	--tertiary-text-50: #000000;
	--tertiary-text-100: #000000;
	--tertiary-text-200: #000000;
	--tertiary-text-300: #ffffff;
	--tertiary-text-400: #ffffff;
	--tertiary-text-500: #ffffff;
	--tertiary-text-600: #ffffff;
	--tertiary-text-700: #ffffff;
	--tertiary-text-800: #ffffff;
	--tertiary-text-900: #ffffff;
	--tertiary-text-950: #ffffff;
}

.dust {
	--textarea-text: #000000;
	--textarea-bg: #eeeeee;

	--primary-50: #f6f6f4;
	--primary-100: #edece9;
	--primary-200: #e3e3dd;
	--primary-300: #dadad2;
	--primary-400: #d1d0c7;
	--primary-500: #c8c7bc;
	--primary-600: #aba998;
	--primary-700: #8e8c75;
	--primary-800: #6b6957;
	--primary-900: #47463a;
	--primary-950: #24231d;

	--primary-text-50: #000000;
	--primary-text-100: #000000;
	--primary-text-200: #000000;
	--primary-text-300: #000000;
	--primary-text-400: #000000;
	--primary-text-500: #000000;
	--primary-text-600: #ffffff;
	--primary-text-700: #ffffff;
	--primary-text-800: #ffffff;
	--primary-text-900: #ffffff;
	--primary-text-950: #ffffff;

	--secondary-50: #e5edef;
	--secondary-100: #cadbdf;
	--secondary-200: #b0c8cf;
	--secondary-300: #96b6be;
	--secondary-400: #7ba4ae;
	--secondary-500: #62929e;
	--secondary-600: #517a84;
	--secondary-700: #416169;
	--secondary-800: #30494f;
	--secondary-900: #203135;
	--secondary-950: #10181a;

	--secondary-text-50: #000000;
	--secondary-text-100: #000000;
	--secondary-text-200: #000000;
	--secondary-text-300: #000000;
	--secondary-text-400: #ffffff;
	--secondary-text-500: #ffffff;
	--secondary-text-600: #ffffff;
	--secondary-text-700: #ffffff;
	--secondary-text-800: #ffffff;
	--secondary-text-900: #ffffff;
	--secondary-text-950: #ffffff;

	--tertiary-50: #dfe8ec;
	--tertiary-100: #bed0d8;
	--tertiary-200: #9eb9c5;
	--tertiary-300: #7da2b1;
	--tertiary-400: #5e8a9d;
	--tertiary-500: #4a6d7c;
	--tertiary-600: #3e5b68;
	--tertiary-700: #324953;
	--tertiary-800: #25373e;
	--tertiary-900: #192429;
	--tertiary-950: #0c1215;

	--tertiary-text-50: #000000;
	--tertiary-text-100: #000000;
	--tertiary-text-200: #000000;
	--tertiary-text-300: #ffffff;
	--tertiary-text-400: #ffffff;
	--tertiary-text-500: #ffffff;
	--tertiary-text-600: #ffffff;
	--tertiary-text-700: #ffffff;
	--tertiary-text-800: #ffffff;
	--tertiary-text-900: #ffffff;
	--tertiary-text-950: #ffffff;
}

widget {
	font-family: "system";
	font-size: 13;
	color: var(--primary-text-500);
	window-background: var(--primary-500);
	window-border: var(--primary-700);
	window-caption-color: var(--primary-500);
	window-caption-text-color: var(--primary-text-500);
}

textlabel {
}

pushbutton {
	noncontent-left: 10;
	noncontent-top: 5;
	noncontent-right: 10;
	noncontent-bottom: 5;
	color: var(--secondary-text-500);
	background-color: var(--secondary-500);
	border-left-color: var(--secondary-600);
	border-top-color: var(--secondary-600);
	border-right-color: var(--secondary-600);
	border-bottom-color: var(--secondary-600);
}

pushbutton.bordeaux {
	background-color: transparent;
	border-left-color: transparent;
	border-right-color: transparent;
	border-top-color: transparent;
	border-bottom-color: transparent;
	border-image-source: url("PushButton.png");
	border-top-width: 2;
	border-bottom-width: 2;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 4;
	border-bottom-image-slice: 4;
	border-left-image-slice: 8;
	border-right-image-slice: 8;
	border-top-image-width: 2;
	border-bottom-image-width: 2;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
}

pushbutton:hover {
	color: var(--secondary-text-400);
	background-color: var(--secondary-400);
}

pushbutton.bordeaux:hover {
	border-image-source: url("PushButtonHover.png");
	background-color: transparent;
}

pushbutton:down {
	color: var(--secondary-text-300);
	background-color: var(--secondary-300);
}

pushbutton.bordeaux:down {
	border-image-source: url("PushButtonPressed.png");
	background-color: transparent;
}

lineedit {
	noncontent-left: 5;
	noncontent-top: 3;
	noncontent-right: 5;
	noncontent-bottom: 3;
	color: var(--textarea-text);
	background-color: var(--textarea-bg);
	border-left-color: var(--secondary-500);
	border-top-color: var(--secondary-500);
	border-right-color: var(--secondary-500);
	border-bottom-color: var(--secondary-500);
	selection-color: var(--tertiary-text-500);
	selection-background: var(--tertiary-500);
	selection-color-nofocus: var(--tertiary-text-400);
	selection-background-nofocus: var(--tertiary-400);
}

lineedit.bordeaux {
	background-color: transparent;
	border-left-color: transparent;
	border-right-color: transparent;
	border-top-color: transparent;
	border-bottom-color: transparent;
	border-image-source: url("TextField.png");
	border-top-width: 2;
	border-bottom-width: 2;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 4;
	border-bottom-image-slice: 4;
	border-left-image-slice: 8;
	border-right-image-slice: 10;
	border-top-image-width: 2;
	border-bottom-image-width: 2;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
}

lineedit.bordeaux:hover {
	background-color: transparent;
	border-left-color: transparent;
	border-right-color: transparent;
	border-top-color: transparent;
	border-bottom-color: transparent;
	border-image-source: url("TextFieldHover.png");
	border-top-width: 2;
	border-bottom-width: 2;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 4;
	border-bottom-image-slice: 4;
	border-left-image-slice: 8;
	border-right-image-slice: 10;
	border-top-image-width: 2;
	border-bottom-image-width: 2;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
}

textedit {
	font-family: "monospace";
	noncontent-left: 8;
	noncontent-top: 8;
	noncontent-right: 8;
	noncontent-bottom: 8;
	color: var(--textarea-text);
	background-color: var(--textarea-bg);
	border-left-color: var(--secondary-500);
	border-top-color: var(--secondary-500);
	border-right-color: var(--secondary-500);
	border-bottom-color: var(--secondary-500);
	selection-color: var(--tertiary-text-500);
	selection-background: var(--tertiary-500);
	selection-color-nofocus: var(--tertiary-text-400);
	selection-background-nofocus: var(--tertiary-400);
}

)xxxx";
// compilers suck!
static const char* theme_default_stylesheet2 = R"xxxx(

listview, treeview {
	noncontent-left: 1;
	noncontent-top: 1;
	noncontent-right: 1;
	noncontent-bottom: 1;
	background-color: var(--textarea-bg);
	border-left-color: var(--secondary-500);
	border-top-color: var(--secondary-500);
	border-right-color: var(--secondary-500);
	border-bottom-color: var(--secondary-500);
}

listview-body, treeview-body {
	color: var(--textarea-text);
	selection-color: var(--tertiary-text-500);
	selection-background: var(--tertiary-500);
	noncontent-top: 2;
	noncontent-left: 8;
	noncontent-bottom: 9;
}

listview-header, treeview-header {
	background-color: var(--primary-600);
	color: var(--primary-text-600);
	noncontent-left: 10;
	noncontent-right: 2;
}

dropdown {
	noncontent-left: 5;
	noncontent-top: 5;
	noncontent-right: 5;
	noncontent-bottom: 5;
	color: var(--textarea-color);
	background-color: var(--textarea-bg);
	border-left-color: var(--secondary-500);
	border-top-color: var(--secondary-500);
	border-right-color: var(--secondary-500);
	border-bottom-color: var(--secondary-500);
	arrow-color: var(--secondary-text-500);
}

dropdown.bordeaux {
	background-color: transparent;
	border-left-color: transparent;
	border-right-color: transparent;
	border-top-color: transparent;
	border-bottom-color: transparent;
	border-image-source: url("TextField.png");
	arrow-image: url("DropdownTriangle.png");
	arrow-image-width: 16;
	arrow-image-height: 16;
	border-top-width: 2;
	border-bottom-width: 2;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 4;
	border-bottom-image-slice: 4;
	border-left-image-slice: 8;
	border-right-image-slice: 8;
	border-top-image-width: 2;
	border-bottom-image-width: 2;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
	color: var(--textarea-text);
}

scrollbar {
	track-color: var(--primary-500);
	thumb-color: var(--secondary-500);
	noncontent-top: 5;
	noncontent-bottom: 5;
}

tabbar {
	spacer-left: 20;
	spacer-right: 20;
	background-color: var(--primary-400);
}

tabbar-tab {
	noncontent-left: 15;
	noncontent-right: 15;
	noncontent-top: 1;
	noncontent-bottom: 1;
	color: var(--primary-text-600);
	background-color: var(--primary-600);
	border-left-color: var(--primary-700);
	border-top-color: var(--primary-700);
	border-right-color: var(--primary-700);
	border-bottom-color: var(--primary-700);
}

tabbar-tab:hover {
	color: var(--primary-text-400);
	background-color: var(--primary-400);
}

tabbar-tab:active {
	color: var(--primary-text-500);
	background-color: var(--primary-500);
	border-left-color: var(--primary-600);
	border-top-color: var(--primary-600);
	border-right-color: var(--primary-600);
	border-bottom-color: transparent;
}

tabbar-spacer {
	noncontent-bottom: 1;
	border-bottom-color: var(--primary-600);
}

tabwidget-stack {
	noncontent-left: 20;
	noncontent-top: 5;
	noncontent-right: 20;
	noncontent-bottom: 5;
}

checkbox-label, radiobutton-label {
	checked-outer-border-color: var(--secondary-500);
	checked-inner-border-color: var(--bgMain);
	checked-color: var(--secondary-text-500);
	unchecked-outer-border-color: var(--secondary-500);
	unchecked-inner-border-color: var(--bgMain);
}

checkbox-label.bordeaux {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("CheckBoxTicked.png");
	checked-align: 0;
	unchecked-image: url("CheckBox.png");
	unchecked-align: 0;
}

checkbox-label.bordeaux:hover {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("CheckBoxTickedHover.png");
	checked-align: 0;
	unchecked-image: url("CheckBoxHover.png");
	unchecked-align: 0;
}

radiobutton-label.bordeaux {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("RadioButtonTicked.png");
	checked-align: 0;
	unchecked-image: url("RadioButton.png");
	unchecked-align: 0;
}

radiobutton-label.bordeaux:hover {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("RadioButtonTickedHover.png");
	checked-align: 0;
	unchecked-image: url("RadioButtonHover.png");
	unchecked-align: 0;
}

menubar {
	background-color: var(--primary-500);
}

menubaritem {
	color: var(--primary-text-500)
}

menubaritem:hover {
	color: var(--primary-text-400);
	background-color: var(--primary-400);
}

menubaritem:down {
	color: var(--primary-text-300);
	background-color: var(--primary-300);
}

menu {
	noncontent-left: 5;
	noncontent-top: 5;
	noncontent-right: 5;
	noncontent-bottom: 5;
	color: var(--fgMain);
	background-color: var(--primary-500);
	border-left-color: var(--secondary-500);
	border-top-color: var(--secondary-500);
	border-right-color: var(--secondary-500);
	border-bottom-color: var(--secondary-500);
}

menuitem {
}

menuitem:hover {
	color: var(--primary-text-400);
	background-color: var(--primary-400);
}

menuitem:down {
	color: var(--primary-text-300);
	background-color: var(--primary-300);
}

toolbar {
	background-color: var(--primary-500);
}

toolbarbutton {
	color: var(--primary-text-500);
	background-color: var(--primary-500);
}

toolbarbutton:hover {
	color: var(--primary-text-400);
	background-color: var(--primary-400);
}

toolbarbutton:down {
	color: var(--primary-text-300);
	background-color: var(--primary-300);
}

statusbar {
	background-color: var(--primary-500);
}

)xxxx";
