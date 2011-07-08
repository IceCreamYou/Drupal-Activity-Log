Drupal.behaviors.activityLogAdmin = function (context) {
  // Make sure we can run context.find().
  var ctxt = $(context);

  // Show/hide grouping settings.
  var handle = function() {
    var val = ctxt.find('input:radio[name="settings[grouping][group_method]"]:checked').val();
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
  handle();
  ctxt.find('input:radio[name="settings[grouping][group_method]"]').change(handle);

  // Show/hide stream owner exposed fields.
  ctxt.find('#edit-settings-stream-owner-id-wrapper').hide();
  ctxt.find('#edit-settings-stream-owner-type-wrapper').hide();
  var fields = Drupal.settings.activity_log.stream_owner_expose_fields;
  $.each(fields, function(key, value) {
    ctxt.find('input:checkbox[name="settings[stream_owner_entity_group]['+ key +']"]').change(function() {
      var shown = new Array();
      $.each(fields, function(k, v) {
        for (var val in v) {
          if ($('input:checkbox[name="settings[stream_owner_entity_group]['+ k +']"]').attr('checked')) {
            if ($.inArray(v[val], shown) == -1) {
              $('#edit-settings-stream-owner-'+ v[val] +'-wrapper').show();
              shown[v[val]] = v[val];
            }
          }
        }
      });
      var f = ['id', 'type'];
      for (var val in f) {
        if (shown[f[val]] == undefined || shown[f[val]] == null) {
          $('#edit-settings-stream-owner-'+ f[val] +'-wrapper').hide();
        }
      }
    });
  });
}
