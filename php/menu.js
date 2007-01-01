function toggleMenu(objID) {
        if (!document.getElementById) return;
        var ob = document.getElementById(objID).style;
        ob.display = (ob.display == 'block')?'none': 'block';
}
