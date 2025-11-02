import { ComponentFixture, TestBed } from '@angular/core/testing';

import { RequestPage } from './request-page';

describe('RequestPage', () => {
  let component: RequestPage;
  let fixture: ComponentFixture<RequestPage>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [RequestPage]
    })
    .compileComponents();

    fixture = TestBed.createComponent(RequestPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
