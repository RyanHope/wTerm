
var kbdLayouts = { };

function getKbdLayout(name, onload) {
	var layout, n;

	if (!kbdLayouts[name]) name = (enyo.fetchDeviceInfo().keyboardAvailable || enyo.fetchDeviceInfo().keyboardSlider) ? 'phone_aux' : 'qwerty_us';
	layout = kbdLayouts[name];

	if (layout.keys) {
		onload(layout.keys);
	} else {
		if (!layout.onload) {
			layout.onload = [];
			n = document.createElement("script");
			n.setAttribute('src', 'src/models/vkblayouts/' + name + '.js');
			n.setAttribute('type', 'text/javascript');
			n.setAttribute('onerror', 'console.error(\'Error loading keyboard layout ' + name + '\')');
		}
		layout.onload.push(onload);
		if (n) document.body.appendChild(n);
	}
}

function kbdLayoutList() {
	var l = [], k;
	for (k in kbdLayouts) {
		if (kbdLayouts.hasOwnProperty(k)) {
			l.push({ caption: kbdLayouts[k].caption, value: k });
		}
	}
	return l;
}

function kbdLayoutLoad(obj) {
	var layout = kbdLayouts[obj.name];
	if (!layout || layout.keys) return;
	layout.keys = keys;
	var l = layout.onload;
	if (l) for (i = 0; i < l.length; i++) {
		enyo.asyncMethod(null, l[i], keys);
	}
	layout.onload = null;
}
