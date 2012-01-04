enyo.kind({
	name: 'wi.Sql',
	kind: enyo.Control,
	published: {
		db:			'',	// holds the database connection
		dbName:		'',	// defaults to app title from appinfo 
		dbDesc:		'',	// defaults to app title + Database
		dbSize:		0	// defaults to 5MB
	},
	
	constructor: function() {
	    this.inherited(arguments);
		if (!this.getDb()) this.open();
	},
	
	open: function() {
		try {
			//this.db = false;
			this.db = openDatabase(
				this.getDbName()	|| enyo.fetchAppInfo().title,
				'', // if we keep this blank we always get the current version
				this.getDbDesc()	|| enyo.fetchAppInfo().title + ' Database',
				this.getDbSize()	|| (5 * 1024 * 1024) // 5MB
			);
		}
		catch(e) { 
			console.log('wi.Sql#openDb', e);
			return false;
		}
		return true;
	},
	
	executeSql: function(tx, sql, values, onSuccess, onError) {
		try {
			if (!sql) {
				console.log('wi.Sql#executeSql:', 'you need sql to execute');
				return false;
			}
			if (enyo.isArray(sql) && enyo.isArray(values) && values.length == 0) {
				//console.log('wi.Sql#executeSql: 1 array-blank');
				for (var s = 0; s < sql.length; s++) {
					tx.executeSql(
						sql[s],
						[],
						onSuccess,
						function (tx, results) { this.error(tx, results, sql[s], [], onError); }.bind(this)
					);
				}
			}
			else if (!enyo.isArray(sql) && enyo.isArray(values) && values.length > 0 && enyo.isArray(values[0])) {
				//console.log('wi.Sql#executeSql: 2 string-arrayofarrays');
				for (var v = 0; v < values.length; v++) {
					tx.executeSql(
						sql,
						values[v],
						onSuccess,
						function (tx, results) { this.error(tx, results, sql, values[v], onError); }.bind(this)
					);
				}
			}
			else if (enyo.isArray(sql) && enyo.isArray(values) && values.length > 0 && enyo.isArray(values[0])) {
				//console.log('wi.Sql#executeSql: 3 array-array');
				for (var s = 0; s < sql.length; s++) {
					tx.executeSql(
						sql[s],
						values[s],
						onSuccess,
						function (tx, results) { this.error(tx, results, sql[s], values[s], onError); }.bind(this)
					);
				}
			}
			else {
				//console.log('wi.Sql#executeSql: 4 string-string, normal');
				tx.executeSql(
					sql,
					values,
					onSuccess,
					function (tx, results) { this.error(tx, results, sql, values, onError); }.bind(this)
				);
			}
		}
		catch(e) { 
			console.log('wi.Sql#executeSql', e, tx, sql, values, onSuccess, onError);
			return false;
		}
		return true;
	},
	executeSqlChain: function(tx, sql, values, onSuccess, onError) {
		try {
			if (!sql) {
				console.log('wi.Sql#executeSql:', 'you need sql to execute');
				return false;
			}
			if (enyo.isArray(sql) && enyo.isArray(values)) {
				//console.log('wi.Sql#executeSqlChain: 1 array-array');
				var linkSql = sql.shift();
				var linkValues = values.shift();
				this.executeSql(
					tx,
					linkSql,
					linkValues,
					function (tx, results) { if (sql.length > 0) { this.executeSqlChain(tx, sql, values, onSuccess, onError); } else { if (onSuccess) onSuccess(); } }.bind(this),
					function (tx, results) { this.error(tx, results, linkSql, linkValues, onError); }.bind(this)
				);
			}
			else {
				console.log('wi.Sql#executeSqlChain: 2 string-array');
				this.executeSql(
					tx,
					sql,
					values,
					onSuccess,
					onError
				);
			}
		}
		catch(e) { 
			//console.log('wi.Sql#executeSqlChain', e, tx, sql, values, onSuccess, onError);
			return false;
		}
		return true;
	},
	
	error: function(tx, results, sql, values, callback) {
		console.log('wi.Sql#error:', results.code, '-', results.message, ':', '"' + sql + '"', values);
		if (callback) callback();
	},
	
	getVersion: function(integer) {
		if (integer) return (isNaN(parseInt(this.getVersion(), 10)) ? 0 : parseInt(this.getVersion(), 10));
		else return this.db.version;
	},
	changeVersion: function(newVersion, sql, onSuccess) {
		try {
			if (this.getVersion() != newVersion &&
				this.db.changeVersion) {
				//console.log('changing version', this.getVersion(), newVersion);
				this.db.changeVersion(this.getVersion(), newVersion, function (tx) {
					this.executeSqlChain(
						tx,
						sql,
						[],
						function() { /*this.open();*/ if (onSuccess) { onSuccess(); } }.bind(this)
					);
				}.bind(this));
				return true;
			}
		}
		catch(e) { 
			console.log('wi.Sql#changeVersion', e, newVersion, sql);
		}
		return false;
	},
	
	query: function(sql, values, onSuccess, onError) {
		//console.log(sql, values, onSuccess, onError);
		this.db.transaction(function (tx) {
			this.executeSql(
				tx,
				sql,
				values || [],
				onSuccess,
				onError
			);
		}.bind(this));
	},
	
	getResults: function(sql, values, onSuccess, onError) {
		this.query(
			sql,
			values,
			function (tx, results) { this.parseResults(tx, results, onSuccess);	}.bind(this),
			onError
		);
	},
	parseResults: function(tx, results, callback) {
		if (results.rows.length > 0) {
			var array = [];
			for (var r = 0; r < results.rows.length; r++) {
				array.push(results.rows.item(r));
			}
			if (callback) callback(array);
		}
		else {
			if (callback) callback([]);
		}
	},
	
	getRow: function(sql, values, onSuccess, onError) {
		this.query(
			sql,
			values,
			function (tx, results) { this.parseRow(tx, results, onSuccess);	}.bind(this),
			onError
		);
	},
	parseRow: function(tx, results, callback) {
		if (results.rows.length > 0) {
			if (callback) callback(results.rows.item(0));
		}
		else {
			if (callback) callback(false);
		}
	},
	
	getVar: function(sql, values, onSuccess, onError) {
		this.query(
			sql,
			values,
			function (tx, results) { this.parseVar(tx, results, onSuccess);	}.bind(this),
			onError
		);
	},
	parseVar: function(tx, results, callback) {
		if (results.rows.length > 0) {
			var variable = false;
			for (var t in results.rows.item(0)) {
				variable = results.rows.item(0)[t];
			}
			if (callback) callback(variable);
		}
		else {
			if (callback) callback(false);
		}
	}
	
});
