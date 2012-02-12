enyo.application.vkbLayouts.unshift({caption: 'Phone Auxiliary', value: 'phone_aux'})
enyo.kind({

	kind: 'vkb',
	name: enyo.application.vkbLayouts[0].value,
	caption: enyo.application.vkbLayouts[0].caption,
	
	layout: [
		[
			{flex:1},
			{symbols: [['Esc',SDLK._ESCAPE]]},
			{flex:1},
			{symbols: [['Tab<br><img src="images/key_tab.png" class="keyImg"/>',SDLK._TAB]]},
			{flex:1},
			{symbols: [['<img src="images/cursorUp.png" class="keyImg"/>',SDLK._UP],null,['<span class="fnBind">PgUp</span>',SDLK._PAGEUP]], extraClasses: 'arrow'},
			{flex:1},
			{symbols: [['<img src="images/cursorDown.png" class="keyImg"/>',SDLK._DOWN],null,['<span class="fnBind">PgDn</span>',SDLK._PAGEDOWN]], extraClasses: 'arrow'},
			{flex:1},
			{symbols: [['<img src="images/cursorLeft.png" class="keyImg"/>',SDLK._LEFT],null,['<span class="fnBind">Home</span>',SDLK._HOME]], extraClasses: 'arrow'},
			{flex:1},
			{symbols: [['<img src="images/cursorRight.png" class="keyImg"/>',SDLK._RIGHT],null,['<span class="fnBind">End</span>',SDLK._END]], extraClasses: 'arrow'},
			{flex:1},
			{symbols: [['Sym',SDLK._MENU]], extraClasses: 'sym'},
			{flex:1}
		]
	]
});