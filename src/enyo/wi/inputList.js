enyo.kind({
	name: 'wi.InputList',
	kind: 'Control',
	
	published: {
		value: [],
		inputHint: ''
	},
	
	events: {
		'onAdd': ''
	},
	
	components: [
	    {name: 'list', kind: 'VirtualRepeater', onSetupRow: 'getItem', components: [
	        {name: 'item', kind: 'SwipeableItem', allowLeft: true, onConfirm: 'inputSwiped', components: [
				{name: 'input', kind: 'Input', autocorrect: false, autoCapitalize: 'lowercase',
					autoWordComplete: false, selectAllOnFocus: true, onkeyup: 'inputChanged',
					components: [{name: 'caption', content: ''}]}
	        ]}
	    ]},
		{name: 'addrow', kind: 'Item', className: 'enyo-single', tapHighlight: true, onclick: 'addClicked', components: [
			{content: 'Add', style: 'text-align:center;'},
		]},
	],
	
	create: function () {
	    this.inherited(arguments);
		this.addClass('wi-input-list');
	},
	
	getItem: function(inSender, inIndex) {
		if (!enyo.isArray(this.value)) this.value = [];
	    if (inIndex < this.value.length) {
	    	if (this.value[inIndex] instanceof Array) {
	    		this.$.input.setValue(this.value[inIndex][0]);
	    		this.$.caption.setContent(this.value[inIndex][1]);
	    	} else {
				this.$.input.setValue(this.value[inIndex]);
			}
			this.$.input.setHint(this.inputHint);
			if (inIndex == 0)	this.$.item.addClass('enyo-first');
			else				this.$.item.addClass('enyo-middle');
	        return true;
	    }
		this.$.addrow.addRemoveClass('enyo-last', (this.value.length > 0));
		this.$.addrow.addRemoveClass('enyo-single', (this.value.length == 0));
	},
	
	inputChanged: function(inSender, inEvent) {
		if (inEvent.rowIndex != null)
			this.value[inEvent.rowIndex] = inSender.getValue();
	},
	inputSwiped: function(inSender, inIndex) {
		if (inIndex != null) {
			this.value.splice(inIndex, 1);
			this.$.list.render();
		}
	},
	
	addClicked: function(inSender, inEvent) {
		if (this.doAdd)
			this.doAdd();
		else
			this.addValue('');
	},
	
	setValue: function(value) {
		this.value = value;
	},
	getValue: function() {
		return this.value;
	},
	
	addValue: function(value) {
		this.value.push(value);
		this.$.list.render();
	}
	
});



