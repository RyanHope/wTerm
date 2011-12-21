enyo.kind({
	
	name: "vkb",
  	kind: 'VFlexBox',
  	flex: 1,
  	width: '100%',
  	
	shift: 1,
	ctrl: 2,
	alt: 4,
	fn: 8,
	caps: 16,
  	
  	published: {
		terminal: null,
		mode: 0
  	},

  	components: [
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'Esc', ontouchstart: 'btnClick'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F1', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F2', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F3', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F4', ontouchstart: 'fnbtnClick'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F5', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F6', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F7', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F8', ontouchstart: 'fnbtnClick'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F9', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F10', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F11', ontouchstart: 'fnbtnClick'},
			{kind: 'vkbKey', className: 'enyo-button key-small', content: 'F12', ontouchstart: 'fnbtnClick'}
		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', content: '!<br>1', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '@<br>2', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '#<br>3', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '$<br>4', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '%<br>5', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '^<br>6', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '&<br>7', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '*<br>8', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '(<br>9', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: ')<br>0', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '_<br>-', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '+<br>=', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'Bksp', flex: 1, ontouchstart: 'btnClick'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', content: 'Tab', flex: 1, ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'q', sym: 113, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'w', sym: 119, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'e', sym: 101, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'r', sym: 114, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 't', sym: 116, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'y', sym: 121, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'u', sym: 117, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'i', sym: 105, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'o', sym: 111, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'p', sym: 112, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: '{<br>[', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '}<br>]', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '|<br>\\', ontouchstart: 'btnClick'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
			{kind: 'vkbKey', content: 'Caps Lock', flex: 1, toggling: true, ontouchstart: 'toggleCaps'},
  			{kind: 'vkbKey', content: 'a', sym: 97, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 's', sym: 115, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'd', sym: 100, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'f', sym: 102, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'g', sym: 103, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'h', sym: 104, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'j', sym: 106, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'k', sym: 107, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'l', sym: 108, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: ':<br>;', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: '\"<br>\'', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'Enter', flex: 1, sym: 13, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', content: 'Shift', flex: 1, sym: 304, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'z', sym: 122, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'x', sym: 120, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'c', sym: 99, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'v', sym: 118, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'b', sym: 98, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'n', sym: 110, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: 'm', sym: 109, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', content: '<<br>,', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'Up', ontouchstart: 'btnClick'},
			{kind: 'vkbKey', content: '><br>.', ontouchstart: 'btnClick'},
			{kind: 'vkbKey', content: 'Shift', style: 'min-width: 120px;', sym: 303, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
			{kind: 'vkbKey', content: 'Ctrl', flex: .6, ontouchstart: 'ctrlDown', ontouchend: 'ctrlUp'},
  			{kind: 'vkbKey', content: 'Fn', ontouchstart: 'fnDown', ontouchend: 'fnUp'},
  			{kind: 'vkbKey', content: 'Alt', ontouchstart: 'altDown', ontouchend: 'altUp'},
  			{kind: 'vkbKey', content: 'Space', flex: 4, ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'Left', ontouchstart: 'btnClick'},
			{kind: 'vkbKey', content: 'Down', ontouchstart: 'btnClick'},
  			{kind: 'vkbKey', content: 'Right', ontouchstart: 'btnClick'},
			{kind: 'vkbKey', content: '~<br>`', ontouchstart: 'btnClick'},
			{kind: 'vkbKey', content: '?<br>/', ontouchstart: 'btnClick'},
  		]},
  	],
  	
  	keyUp: function(inSender) {
  		this.keyPress(0, inSender.sym, inSender.content)
  	},
  	
  	keyDown: function(inSender) {
  		this.keyPress(1, inSender.sym, inSender.content)
  	},
  	
  	keyPress: function(state,sym,content) {
  		var key = content.split('<br>').reverse()
  		var unicode = key[0]
  		switch (key[0]) {
  			case 'Enter':
  				unicode = '\x0d'
  				break
  		}
  		if (state)
  			this.terminal.keyDown(sym, unicode)
  		else
  			this.terminal.keyUp(sym, unicode)
  	},
  	
	btnClick: function(inSender, inEvent) {
		var key = inSender.content.split('<br>').reverse()
		switch (key[0]) {
			case 'Tab':
				this.terminal.write('\x09')
				break
			case 'Up':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OA') : this.terminal.write('\033[A')					
				break
			case 'Down':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OB') : this.terminal.write('\033[B')
				break
			case 'Left':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OD') : this.terminal.write('\033[D')
				break
			case 'Right':
				(this.terminal.modes['appkeys']) ? this.terminal.write('\033OC') : this.terminal.write('\033[C')
				break
			case 'Enter':
				if (this.terminal.modes['newline']) {
					this.terminal.write('\x0d')
					this.terminal.write('\x0a')
				} else
					this.terminal.write('\x0d')
				break
			case 'Esc':
				this.terminal.write('\x1b')
				break
			case 'Bksp':
				this.terminal.write('\x7f')
				break
			case 'Space':
				this.terminal.write(' ')
				break
			default:
				if (this.mode == this.ctrl) {
					var base = key[0].toUpperCase().charCodeAt(0)
					if (key.length>1 && (key[1]=="@" || key[1]=="~" || key[1]=="^" || key[1]=="?" || key[1]=="_"))
						base = key[1].charCodeAt(0)
					if (base > 63 && base < 96)
						this.terminal.write(String.fromCharCode(base-64))
				} else if (this.mode == this.caps || this.mode == this.shift) {
					if (key.length>1) {
						this.terminal.write(key[1])
					} else {
						this.terminal.write(key[0].toUpperCase())
					}
				} else {
					this.terminal.write(key[0])
				}
		}
		
	},
	
	// Handle special modifier key states here

	toggleCaps: function(inSender, inEvent) {
		if (inSender.down )
			this.mode += this.caps
		else
			this.mode -= this.caps
	},
	
	shiftDown: function() {
		this.mode += this.shift
	},
	
	shiftUp: function() {
		this.mode -= this.shift
	},
	
	fnDown: function() {
		this.mode += this.fn
	},
	
	fnUp: function() {
		this.mode -= this.fn
	},
	
	altDown: function() {
		this.mode += this.alt
	},
	
	altUp: function() {
		this.mode -= this.alt
	},
	
	ctrlDown: function() {
		this.mode += this.ctrl
	},
	
	ctrlUp: function() {
		this.mode -= this.ctrl
	}
	
})
