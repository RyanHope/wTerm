var Utils = {
	defer: function (func /*, args */) {
        var args = Array.prototype.slice.call(arguments, 1);
        return window.setTimeout(function () {
            return func.apply(undefined, args);
        }, 10);
    },
    onDevice: function () {
		return (window && window.PalmSystem);
    },
}

Array.prototype.detect = Array.prototype.detect || function (inIterator, inContext) {
	for (var i = 0, len = this.length; i < len; i += 1) {
		if (inIterator.call(inContext, this[i])) {
			return this[i];
		}
	}
	return undefined;
};