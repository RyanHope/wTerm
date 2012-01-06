enyo.kind({
	
	name: "vkbsmall",
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
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'Esc', sym: 27, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F1', sym: 282, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F2', sym: 283, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F3', sym: 294, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F4', sym: 285, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F5', sym: 286, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F6', sym: 287, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F7', sym: 288, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F8', sym: 289, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{flex:1},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F9', sym: 290, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F10', sym: 291, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F11', sym: 292, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2-small', content: 'F12', sym: 293, ontouchstart: 'keyDown', ontouchend: 'keyUp'}
		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '!<br>1', sym: 48, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '@<br>2', sym: 49, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '#<br>3', sym: 50, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '$<br>4', sym: 51, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '%<br>5', sym: 52, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '^<br>6', sym: 53, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '&<br>7', sym: 54, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '*<br>8', sym: 55, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '(<br>9', sym: 56, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: ')<br>0', sym: 48, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '_<br>-', sym: 45, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '+<br>=', sym: 61, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Bksp', flex: 1, sym: 8, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Tab', flex: 1, sym: 9, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'q', sym: 113, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'w', sym: 119, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'e', sym: 101, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'r', sym: 114, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 't', sym: 116, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'y', sym: 121, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'u', sym: 117, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'i', sym: 105, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'o', sym: 111, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'p', sym: 112, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '{<br>[', sym: 91, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '}<br>]', sym: 93, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '|<br>\\', sym: 92, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Caps Lock', flex: 1, toggling: true, ontouchstart: 'toggleCaps'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'a', sym: 97, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 's', sym: 115, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'd', sym: 100, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'f', sym: 102, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'g', sym: 103, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'h', sym: 104, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'j', sym: 106, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'k', sym: 107, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'l', sym: 108, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: ':<br>;', sym: 59, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '\"<br>\'', sym: 39, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Enter', flex: 1, sym: 13, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Shift', style: 'min-width: 90px;', sym: 304, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'z', sym: 122, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'x', sym: 120, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'c', sym: 99, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'v', sym: 118, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'b', sym: 98, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'n', sym: 110, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'm', sym: 109, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: '<<br>,', sym: 44, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Up', sym: 273, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2', content: '><br>.', sym: 46, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Shift', style: 'min-width: 80px;', flex: 1, sym: 303, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  		]},
		{layoutKind: 'HFlexLayout', pack: 'end', components: [
			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Ctrl', style: 'min-width: 50px;', sym: 306, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Fn', sym: 313, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Alt', sym: 308, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Space', flex: 3.8, sym: 32, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Left', sym: 276, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Down', sym: 274, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
  			{kind: 'vkbKey', className: 'enyo-button key2', content: 'Right', sym: 275, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2', content: '~<br>`', sym: 96, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
			{kind: 'vkbKey', className: 'enyo-button key2', content: '?<br>/', sym: 47, ontouchstart: 'keyDown', ontouchend: 'keyUp'},
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
  				unicode = '\x0D'
  				break
  			case 'Bksp':
  				unicode = '\x7f'
  				break
  			case 'Esc':
  				unicode = '\x1B'
  				break
  			case 'Tab':
  				unicode = '\x09'
  				break
  			case 'Space':
  				unicode = ' '
  				break
  			case 'Shift': case 'Ctrl': case 'Fn': case 'Alt': case 'Up': case 'Down':
  			case 'Left': case 'Right': case 'F1': case 'F2': case 'F3':
  			case 'F4': case 'F5': case 'F6': case 'F7': case 'F8':
  			case 'F9': case 'F10': case 'F11': case 'F12':
  				unicode = null
  				break
  		}
  		if (state)
  			this.terminal.keyDown(sym, unicode)
  		else
  			this.terminal.keyUp(sym, null)
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
