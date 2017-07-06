/*
 *  lion-remote.js
 * 
 *  Created by Diego Belfiore on 8/29/2011
 *  2011 Tatami Software. All Rights Reserved.
 *
 *  Partner system.
 */

//
//  jQuery document bindings: build the view frame and display items.
//
$(document).ready(function () {
    TSAddStylesheets();
    TSBuildViewFrame();
    TSFetchItems();
    TSTearDownEmbedTag();
});

//
//  TSFetchItems
//
//  Fetches the items specified in #item-list and displays them.
//
var TSFetchItems = function () {
    var applicationKey = TSGetApplicationKey();
    var itemList = TSGetItemListFetchQueryString();
    var fetchUrl = 'http://www.oksolar.com/lion/api/fetchitems?applicationKey=' + applicationKey + '&' + itemList;
    $.ajax({
        type: 'GET',
        dataType: 'jsonp',
        url: fetchUrl,
        jsonpCallback: 'TSDisplayItems',
        error: function (jqXHR, textStatus, errorThrown) { alert(textStatus); }
    });
};

//
//  TSDisplayItmes
//  + fetchedItems: a JSON list of items
//
//  Our callback for TSFetchItems ajax call.
//
var TSDisplayItems = function (fetchedItems) {
    $('#ts-lr-progress-view').remove();
    $.each(fetchedItems, function (index, value) {
        $('#json-list').append(TSViewForItemSummary(value));
    });
    $('#json-list').append('<div class=\"ts-lr-divisor-bar\"></div>');
}

//
//  TSFetchItem
//
//  Fetch an item and display it in full view.
//
var TSFetchItem = function (ID) {
    var appKey = TSGetApplicationKey();
    var fetchUrl = 'http://www.oksolar.com/lion/api/fetchitem?applicationKey=' + appKey + '&productID=' + ID;
    var oksolar = $.ajax({
        type: 'GET',
        dataType: 'jsonp',
        url: fetchUrl,
        jsonpCallback: 'TSDisplayItem',
        error: function (jqXHR, textStatus, errorThrown) { alert(textStatus); }
    });
};

//
//  TSDisplayItem
//  + fetchedItem
//
//  Callback for TSFetchItem
//
var TSDisplayItem = function(fetchedItem) { 
    $('#single-item-view').html(TSViewForItem(fetchedItem)); 
};

//
//  TSGetItemListFetchQueryString
//
//  Get the list of items to display from the #item-list tag and builds a request query string
//  to send through ajax.
//
//      [ 00188, 002500 ]  =>  productID=00188&productID=002500
//
var TSGetItemListFetchQueryString = function () {
    var queryString = '';
    var itemList = TSGetItemList();
    var itemsToFetch = itemList.split(',');
    $.each(itemsToFetch, function (index, value) {
        queryString += 'productID=' + value + '&';
    });
    return queryString;
};

//
//  TSGetItemList
//
//  Retrieves the comma-separated item list from the embed tag.
//
var TSGetItemList = function () {
    var embedTag = $('#item-list').attr('title');
    var tagComponents = embedTag.split(';');
    return tagComponents[1];
};

//
//  TSGetApplicationKey
//
//  Retrieves application key from hidden tag.
//
var TSGetApplicationKey = function () {
    var appKey = $('#application-key').val();
    return appKey;
};

//
//  TSAddStylesheets
//
//  Adds the styles we need.
//
var TSAddStylesheets = function() {
  $('head').append('<link rel=\"Stylesheet\" href=\"http://www.oksolar.com/lion/content/lr-min-style.css\" />');
};

//
//  TSTearDownEmbedTag
//
//  Tears down the original embedding tag once our item view is built.
//
var TSTearDownEmbedTag = function () {
    $('#item-list').remove();
};

//
//  TSBuildViewFrame
//
//  Lays out the divs we use to display the list of items and full item view.
//  We also add the top-header and progress indicator.
//
var TSBuildViewFrame = function () {
    $('<div id=\"ts-lr-view-frame\"></div>').insertAfter('#item-list');
    $('#ts-lr-view-frame').append(TSBuildApplicationKeyTag());
    $('#ts-lr-view-frame').append('<div id=\"single-item-view\"></div>');
    $('#ts-lr-view-frame').append('<div id=\"json-list\"></div>');
    $('#json-list').append('<div id=\"ts-lr-top-header\"><span id=\"ts-lr-top-header-logo\"><strong>OkSolar.com</strong> Partner System</span>.</div>');
    $('#json-list').append('<div id=\"ts-lr-progress-view\"></div>');
};

//
//  TSBuildApplicationKeyTag
//
//  Retrieves the application key from the embedding tag and returns a hidden tag to store the app key
//  on the page for future use. This function will only work the first time it's used--before the embed 
//  tag is torn down.
//
var TSBuildApplicationKeyTag = function () {
    var embedTag = $('#item-list').attr('title');
    var tagComponents = embedTag.split(';');
    var appKeyTag = '<input type=\"hidden\" id=\"application-key\" value=\"' + tagComponents[0] + '\" >';
    return appKeyTag;
};

//
//  TSViewForItem
//    + ID: ProductID for the item to be displayed.
//
//  Displays a full view for a single item with 'buy' or 'quote' buttons.
//
var TSViewForItem = function (item) {
    var div = '<div class=\"ts-lr-divisor-bar\"></div>';
    div += '<h1>' + item['Name'] + '</h1>';

    div += '<div id=\"ts-lr-buy-buttons\">';
    if (!item["IsQuoteItem"]) {
        div += '  <div id=\"ts-lr-display-price\">Price: $' + item['Price'] + '</div>';
    }
    div += '    <div id=\"ts-lr-request-quote\">';
    div += '      <form action=\"http://www.oksolar.com/lion/cart/RedirectFromPartner\" method=\"post\">';
    div += '        <input type=\"hidden\" id=\"applicationKey\" name=\"applicationKey\" value=\"' + TSGetApplicationKey() + '\" />';
    div += '        <input type=\"hidden\" id=\"productID\" name=\"productID\" value=\"' + item['ProductID'] + '\" />';
    if (item["IsQuoteItem"]) {
        div += '        <input type=\"submit\" class=\"ts-lr-strong-button\" value=\"+ Request Quote\" />';
    } else {
        div += '        <input type=\"submit\" class=\"ts-lr-strong-button\" value=\"+ Buy or Request Quote\" />';
    }
    div += '      </form>';
    div += '  </div>';
    div += '</div>';

    div += '<div id=\"ts-lr-display-image\">';
    div += '  <img src=\"' + item['ImagePath'] + '\">';
    div += '  <div id=\"ts-lr-display-extra-info\">';
    div += '    Item no.: <strong>' + item['ProductID'] + '</strong><br />';
    div += '    Manufactured by <strong>' + item["Manufacturer"] + '</strong><br />';
    div += '  </div>';
    div += '</div>';

    div += '<div id=\"ts-lr-display-content-frame\">' + item['Description'] + '</div>';

    return div;
};

//
//  TSViewForItemSummary
//    + item: Item object (fetched through ajax) to display.
//
//  Displays items in item-summary rows.
//
var TSViewForItemSummary = function(item) {
  var div = '<div class=\"ts-lr-item\">';

  div += '  <div class=\"ts-lr-item-image\">';
  div += '    <img src=\"' + item["ImagePath"] + '\">';
  div += '  </div>';

  div += '  <div class=\"ts-lr-item-price\">';
  if (item["IsQuoteItem"]) {
    div += '    <img src=\"http://oksolar.com/lion/content/icons/information.png\" alt=\"Request Quote\" title=\"Request Quote\">';
  } else {
    div += '    <span class=\"ts-lr-price-listed\">$' + item["Price"] + '</span>';
  }
  div += '  </div>';

  div += '  <div class=\"ts-lr-item-main\">';
  div += '    <h3><a href=\"http://www.oksolar.com/lion/item/' + item["ProductID"] + '\"';
  div += '      onclick=\"TSFetchItem(\'' + item["ProductID"] + '\'); return false;\">';
  div +=        item["Name"];
  div += '    </a></h3>';
  div += '    <strong>Item no. ' + item["ProductID"] + '</strong><br />';
  div += '    <strong>Manufacturer: ' + item["Manufacturer"] + '</strong>';
  div += '    <p class=\"ts-lr-item-description\">' + item["Description"] + '</p>';
  div += '  </div>';  

  div += '</div>';
  return div;
};
