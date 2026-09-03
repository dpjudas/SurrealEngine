#pragma once

static const char* theme_stylesheet = R"xxxx(

.dark {
	--textarea-text: #ffffff;
	--textarea-bg: #111111;

	--primary-50: #4F5570;
	--primary-100: #494F69;
	--primary-200: #42475E;
	--primary-300: #3D4257;
	--primary-400: #363A4D;
	--primary-500: #313546;
	--primary-600: #2D3040;
	--primary-700: #272A38;
	--primary-800: #252936;
	--primary-900: #222530;
	--primary-950: #1E212B;

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

	--secondary-50: #6670A3;
	--secondary-100: #6C77AD;
	--secondary-200: #707AB3;
	--secondary-300: #737EB8;
	--secondary-400: #7B87C4;
	--secondary-500: #808CCC;
	--secondary-600: #8B98DE;
	--secondary-700: #93A1EB;
	--secondary-800: #99A8F5;
	--secondary-900: #9CABFA;
	--secondary-950: #A0AFFF;

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

	--tertiary-50: #8C6407;
	--tertiary-100: #946A08;
	--tertiary-200: #A17308;
	--tertiary-300: #B07E09;
	--tertiary-400: #BD870A;
	--tertiary-500: #C48D0A;
	--tertiary-600: #CF940B;
	--tertiary-700: #D79A0B;
	--tertiary-800: #E0A10B;
	--tertiary-900: #E6A40C;
	--tertiary-950: #EBA80C;

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

.light {
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

launcher-banner {
	background-color: #252730;
}

textlabel {
}

pushbutton {
	noncontent-left: 10;
	noncontent-top: 5;
	noncontent-right: 10;
	noncontent-bottom: 5;
	color: #000000;
	border-image-source: url("theme/PushButton.png");
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
	border-image-source: url("theme/PushButtonHover.png");
}

pushbutton:down {
	color: #000000;
	border-image-source: url("theme/PushButtonPressed.png");
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
	selection-color: var(--secondary-text-500);
	selection-background: var(--secondary-500);
	selection-color-nofocus: var(--secondary-text-400);
	selection-background-nofocus: var(--secondary-400);
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
	selection-color: var(--secondary-text-500);
	selection-background: var(--secondary-500);
	selection-color-nofocus: var(--secondary-text-400);
	selection-background-nofocus: var(--secondary-400);
}

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
	selection-color: #ffffff;
	selection-background: var(--secondary-500);
	noncontent-top: 2;
	noncontent-left: 8;
	noncontent-right: 8;
	noncontent-bottom: 9;
}

listview-header, treeview-header {
	noncontent-left: 10;
	noncontent-right: 2;
	noncontent-top: 1;
	noncontent-bottom: 1;
	color: #000000;
	border-image-source: url("theme/ViewHeader.png");
	border-top-width: 4;
	border-bottom-width: 4;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 8;
	border-bottom-image-slice: 8;
	border-left-image-slice: 8;
	border-right-image-slice: 8;
	border-top-image-width: 4;
	border-bottom-image-width: 4;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
}

listview-item {
	border-image-source: url("theme/ListViewItemSelected.png");
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

dropdown {
	noncontent-left: 5;
	noncontent-top: 5;
	noncontent-right: 5;
	noncontent-bottom: 5;
	border-image-source: url("theme/TextField.png");
	arrow-image: url("theme/DropdownTriangle.png");
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
	color: #000000;
}

scrollbar {
}

scrollbutton-up, scrollbutton-down, scrollbutton-left, scrollbutton-right, scrollthumb-vert, scrollthumb-horz, scrolltrack-vert, scrolltrack-horz {
	border-top-width: 4;
	border-bottom-width: 4;
	border-left-width: 4;
	border-right-width: 4;
	border-top-image-slice: 8;
	border-bottom-image-slice: 8;
	border-left-image-slice: 8;
	border-right-image-slice: 8;
	border-top-image-width: 4;
	border-bottom-image-width: 4;
	border-left-image-width: 4;
	border-right-image-width: 4;
	border-center-image-slice: true;
}

scrollbutton-up {
	border-image-source: url("theme/ScrollbarUpNormal.png");
}

scrollbutton-up:hover {
	border-image-source: url("theme/ScrollbarUpHover.png");
}

scrollbutton-up:down {
	border-image-source: url("theme/ScrollbarUpPressed.png");
}

scrollbutton-down {
	border-image-source: url("theme/ScrollbarDownNormal.png");
}

scrollbutton-down:hover {
	border-image-source: url("theme/ScrollbarDownHover.png");
}

scrollbutton-down:down {
	border-image-source: url("theme/ScrollbarDownPressed.png");
}

scrollbutton-left {
	border-image-source: url("theme/ScrollbarLeftNormal.png");
}

scrollbutton-left:hover {
	border-image-source: url("theme/ScrollbarLeftHover.png");
}

scrollbutton-left:down {
	border-image-source: url("theme/ScrollbarLeftPressed.png");
}

scrollbutton-right {
	border-image-source: url("theme/ScrollbarRightNormal.png");
}

scrollbutton-right:hover {
	border-image-source: url("theme/ScrollbarRightHover.png");
}

scrollbutton-right:down {
	border-image-source: url("theme/ScrollbarRightPressed.png");
}

scrollthumb-vert, scrollthumb-horz {
	border-image-source: url("theme/ScrollbarThumbNormal.png");
	grip-image: url("theme/ScrollbarGripNormal.png");
	grip-image-width: 16;
	grip-image-height: 16;
}

scrollthumb-vert:hover, scrollthumb-horz:hover {
	border-image-source: url("theme/ScrollbarThumbHover.png");
	grip-image: url("theme/ScrollbarGripHover.png");
}

scrollthumb-vert:down, scrollthumb-horz:down {
	border-image-source: url("theme/ScrollbarThumbPressed.png");
	grip-image: url("theme/ScrollbarGripHover.png");
}

scrolltrack-vert, scrolltrack-horz {
	border-image-source: url("theme/ScrollbarTrack.png");
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

checkbox-label {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("theme/CheckBoxTicked.png");
	checked-align: 0;
	unchecked-image: url("theme/CheckBox.png");
	unchecked-align: 0;
}

checkbox-label:hover {
	checked-image: url("theme/CheckBoxTickedHover.png");
	unchecked-image: url("theme/CheckBoxHover.png");
}

radiobutton-label {
	checked-image-width: 16;
	checked-image-height: 16;
	checked-image: url("theme/RadioButtonTicked.png");
	checked-align: 0;
	unchecked-image: url("theme/RadioButton.png");
	unchecked-align: 0;
}

radiobutton-label:hover {
	checked-image: url("theme/RadioButtonTickedHover.png");
	unchecked-image: url("theme/RadioButtonHover.png");
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
