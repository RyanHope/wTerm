enyo.application.vkbLayouts.unshift({caption: 'Tablet Auxiliary', value: 'tablet_aux'})
enyo.kind({

	kind: 'vkb',
	name: enyo.application.vkbLayouts[0].value,
	caption: enyo.application.vkbLayouts[0].caption,
	
	layout: [
		[
			{symbols: [['Esc',SDLK._ESCAPE]], small: 1, extraClasses: 'escape'},
			{flex:1},
			{symbols: [['F1',SDLK._F1]], small: 1},
			{symbols: [['F2',SDLK._F2]], small: 1},
			{symbols: [['F3',SDLK._F3]], small: 1},
			{symbols: [['F4',SDLK._F4]], small: 1},
			{flex:1},
			{symbols: [['F5',SDLK._F5]], small: 1},
			{symbols: [['F6',SDLK._F6]], small: 1},
			{symbols: [['F7',SDLK._F7]], small: 1},
			{symbols: [['F8',SDLK._F8]], small: 1},
			{flex:1},
			{symbols: [['F9',SDLK._F9]], small: 1},
			{symbols: [['F10',SDLK._F10]], small: 1},
			{symbols: [['F11',SDLK._F11]], small: 1},
			{symbols: [['F12',SDLK._F12]], small: 1},
			{flex:1},
			{symbols: [['Sym',SDLK._MENU]], small: 1, extraClasses: 'sym'},
		]
	]
})