Drupal.behaviors.activityLogAdmin = function (context) {
  // Make sure we can run context.find().
  var ctxt = $(context);

  // Show/hide grouping settings.
  var $group_method = ctxt.find('input:radio[name="settings[grouping][group_method]"]:checked');
  var handle_gm = function() {
    var val = $group_method.val();
    if (val == 'target_action') {
      ctxt.find('#edit-settings-grouping-group-interval-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-max-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-summary-wrapper').hide();
      ctxt.find('#edit-settings-grouping-collapse-method-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-template-wrapper').show();
      ctxt.find('#activity-log-admin-description').hide();
    }
    else if (val == 'action' || val == 'user_action') {
      ctxt.find('#edit-settings-grouping-group-interval-wrapper').show();
      ctxt.find('#edit-settings-grouping-group-max-wrapper').show();
      ctxt.find('#edit-settings-grouping-group-summary-wrapper').show();
      ctxt.find('#edit-settings-grouping-collapse-method-wrapper').show();
      ctxt.find('#edit-settings-grouping-group-template-wrapper').show();
      ctxt.find('#activity-log-admin-description').show();
    }
    else if (val == 'none') {
      ctxt.find('#edit-settings-grouping-group-interval-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-max-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-summary-wrapper').hide();
      ctxt.find('#edit-settings-grouping-collapse-method-wrapper').hide();
      ctxt.find('#edit-settings-grouping-group-template-wrapper').hide();
    }
  };
  handle_gm();
  ctxt.find('input:radio[name="settings[grouping][group_method]"]').change(handle_gm);

  // Show/hide stream owner exposed fields.
  var fields = Drupal.settings.activity_log.stream_owner_expose_fields;
  var handle_so = function() {
    var shown = new Array();
    $.each(fields, function(k, v) {
      for (var val in v) {
        if (ctxt.find('input:checkbox[name="settings[visibility][stream_owner_entity_group]['+ k +']"]').attr('checked')) {
          if ($.inArray(v[val], shown) == -1) {
            var type = 'acting-uid';
            if (v[val] == 'id') {
              type = 'visibility-stream-owner-id';
            }
            else if (v[val] == 'type') {
              type = 'visibility-stream-owner-type';
            }
            ctxt.find('#edit-settings-'+ type +'-wrapper').show();
            shown[v[val]] = v[val];
          }
        }
      }
    });
    var f = ['id', 'type', 'acting_uid'];
    for (var val in f) {
      if (shown[f[val]] == undefined || shown[f[val]] == null) {
        var type = 'acting-uid';
        if (f[val] == 'id') {
          type = 'visibility-stream-owner-id';
        }
        else if (f[val] == 'type') {
          type = 'visibility-stream-owner-type';
        }
        if (f[val] != 'acting_uid' || $group_method.val() != 'user_action') {
          ctxt.find('#edit-settings-'+ type +'-wrapper').hide();
        }
      }
    }
  };
  handle_so();
  $.each(fields, function(key, value) {
    ctxt.find('input:checkbox[name="settings[visibility][stream_owner_entity_group]['+ key +']"]').change(handle_so);
  });
}
